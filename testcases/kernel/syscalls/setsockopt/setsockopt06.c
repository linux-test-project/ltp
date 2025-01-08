// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * [Description]
 *
 * CVE-2016-8655
 *
 * Check for race condition between packet_set_ring() and tp_version. On some
 * kernels, this may lead to use-after-free.
 *
 * Kernel crash fixed in 4.9
 * 84ac7260236a ("packet: fix race condition in packet_set_ring")
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "tst_test.h"
#include "tst_fuzzy_sync.h"
#include "lapi/if_packet.h"
#include "lapi/if_ether.h"

static int sock = -1;
static unsigned int pagesize;
static struct tst_fzsync_pair fzsync_pair;

static void setup(void)
{
	pagesize = SAFE_SYSCONF(_SC_PAGESIZE);
	tst_setup_netns();

	fzsync_pair.exec_loops = 100000;
	tst_fzsync_pair_init(&fzsync_pair);
}

static void *thread_run(void *arg)
{
	int ret;
	struct tpacket_req3 req = {
		.tp_block_size = pagesize,
		.tp_block_nr = 1,
		.tp_frame_size = pagesize,
		.tp_frame_nr = 1,
		.tp_retire_blk_tov = 100
	};

	while (tst_fzsync_run_b(&fzsync_pair)) {
		tst_fzsync_start_race_b(&fzsync_pair);
		ret = setsockopt(sock, SOL_PACKET, PACKET_RX_RING, &req,
			sizeof(req));
		tst_fzsync_end_race_b(&fzsync_pair);

		if (!ret)
			tst_fzsync_pair_add_bias(&fzsync_pair, -10);
	}

	return arg;
}

static void run(void)
{
	int val1 = TPACKET_V1, val3 = TPACKET_V3;

	tst_fzsync_pair_reset(&fzsync_pair, thread_run);

	while (tst_fzsync_run_a(&fzsync_pair)) {
		sock = SAFE_SOCKET(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
		TEST(setsockopt(sock, SOL_PACKET, PACKET_VERSION, &val3,
			sizeof(val3)));

		if (TST_RET == -1 && TST_ERR == EINVAL)
			tst_brk(TCONF | TTERRNO, "TPACKET_V3 not supported");

		if (TST_RET) {
			tst_brk(TBROK | TTERRNO,
				"setsockopt(PACKET_VERSION, TPACKET_V3");
		}

		tst_fzsync_start_race_a(&fzsync_pair);
		setsockopt(sock, SOL_PACKET, PACKET_VERSION, &val1,
			sizeof(val1));
		tst_fzsync_end_race_a(&fzsync_pair);
		SAFE_CLOSE(sock);
	}

	/* setsockopt(PACKET_RX_RING) created a 100ms timer. Wait for it. */
	usleep(300000);

	if (tst_taint_check()) {
		tst_res(TFAIL, "Kernel is vulnerable");
		return;
	}

	tst_res(TPASS, "Nothing bad happened, probably");
}

static void cleanup(void)
{
	tst_fzsync_pair_cleanup(&fzsync_pair);

	if (sock >= 0)
		SAFE_CLOSE(sock);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.runtime = 270,
	.taint_check = TST_TAINT_W | TST_TAINT_D,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_USER_NS=y",
		"CONFIG_NET_NS=y",
		NULL
	},
	.save_restore = (const struct tst_path_val[]) {
		{"/proc/sys/user/max_user_namespaces", "1024", TST_SR_SKIP},
		{}
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "84ac7260236a"},
		{"CVE", "2016-8655"},
		{}
	}
};
