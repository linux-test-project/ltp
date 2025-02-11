// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * CVE-2017-1000111
 *
 * Check for race condition between packet_set_ring() and tp_reserve.
 * The race allows you to set tp_reserve bigger than ring buffer size.
 * While this will cause truncation of all incoming packets to 0 bytes,
 * sanity checks in tpacket_rcv() prevent any exploitable buffer overflows.
 *
 * Race fixed in v4.13
 * c27927e372f0 ("packet: fix tp_reserve race in packet_set_ring")
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

	/*
	 * Reproducing the bug on unpatched system takes <15 loops. The test
	 * is slow and the bug is mostly harmless so don't waste too much
	 * time.
	 */
	fzsync_pair.exec_loops = 500;
	tst_fzsync_pair_init(&fzsync_pair);
}

static void *thread_run(void *arg)
{
	unsigned int val = 1 << 30;

	while (tst_fzsync_run_b(&fzsync_pair)) {
		tst_fzsync_start_race_b(&fzsync_pair);
		setsockopt(sock, SOL_PACKET, PACKET_RESERVE, &val, sizeof(val));
		tst_fzsync_end_race_b(&fzsync_pair);
	}

	return arg;
}

static void run(void)
{
	unsigned int val, version = TPACKET_V3;
	socklen_t vsize = sizeof(val);
	struct tpacket_req3 req = {
		.tp_block_size = pagesize,
		.tp_block_nr = 1,
		.tp_frame_size = pagesize,
		.tp_frame_nr = 1,
		.tp_retire_blk_tov = 100
	};

	tst_fzsync_pair_reset(&fzsync_pair, thread_run);

	while (tst_fzsync_run_a(&fzsync_pair)) {
		sock = SAFE_SOCKET(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
		TEST(setsockopt(sock, SOL_PACKET, PACKET_VERSION, &version,
			sizeof(version)));

		if (TST_RET == -1 && TST_ERR == EINVAL)
			tst_brk(TCONF | TTERRNO, "TPACKET_V3 not supported");

		if (TST_RET) {
			tst_brk(TBROK | TTERRNO,
				"setsockopt(PACKET_VERSION, TPACKET_V3");
		}

		tst_fzsync_start_race_a(&fzsync_pair);
		TEST(setsockopt(sock, SOL_PACKET, PACKET_RX_RING, &req,
			sizeof(req)));
		tst_fzsync_end_race_a(&fzsync_pair);

		SAFE_GETSOCKOPT(sock, SOL_PACKET, PACKET_RESERVE, &val, &vsize);
		SAFE_CLOSE(sock);

		if (TST_RET == -1 && TST_ERR == EINVAL) {
			tst_fzsync_pair_add_bias(&fzsync_pair, 1);
			continue;
		}

		if (TST_RET) {
			tst_brk(TBROK | TTERRNO,
				"Invalid setsockopt() return value");
		}

		if (val > req.tp_block_size) {
			tst_res(TFAIL, "PACKET_RESERVE checks bypassed");
			return;
		}
	}

	tst_res(TPASS, "Cannot reproduce bug");
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
	.runtime = 150,
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
		{"linux-git", "c27927e372f0"},
		{"CVE", "2017-1000111"},
		{}
	}
};
