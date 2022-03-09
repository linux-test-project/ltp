// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 SUSE LLC
 * Author: Marcos Paulo de Souza <mpdesouza@suse.com>
 * LTP port: Martin Doucha <mdoucha@suse.cz>
 */

/*\
 * [Description]
 *
 * Check for possible double free of rx_owner_map after switching packet
 * interface versions aka CVE-2021-22600.
 *
 * Kernel crash fixed in:
 *
 *  commit ec6af094ea28f0f2dda1a6a33b14cd57e36a9755
 *  Author: Willem de Bruijn <willemb@google.com>
 *  Date:   Wed Dec 15 09:39:37 2021 -0500
 *
 *  net/packet: rx_owner_map depends on pg_vec
 *
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sched.h>

#include "tst_test.h"
#include "lapi/if_packet.h"

static int sock = -1;
static unsigned int pagesize;

static void setup(void)
{
	int real_uid = getuid();
	int real_gid = getgid();

	pagesize = SAFE_SYSCONF(_SC_PAGESIZE);
	SAFE_TRY_FILE_PRINTF("/proc/sys/user/max_user_namespaces", "%d", 10);

	SAFE_UNSHARE(CLONE_NEWUSER);
	SAFE_UNSHARE(CLONE_NEWNET);
	SAFE_FILE_PRINTF("/proc/self/setgroups", "deny");
	SAFE_FILE_PRINTF("/proc/self/uid_map", "0 %d 1", real_uid);
	SAFE_FILE_PRINTF("/proc/self/gid_map", "0 %d 1", real_gid);
}

static void run(void)
{
	unsigned int version = TPACKET_V3;
	struct tpacket_req3 req = {
		.tp_block_size = 4 * pagesize,
		.tp_block_nr = 256,
		.tp_frame_size = TPACKET_ALIGNMENT << 7,
		.tp_retire_blk_tov = 64,
		.tp_feature_req_word = TP_FT_REQ_FILL_RXHASH
	};

	req.tp_frame_nr = req.tp_block_size * req.tp_block_nr;
	req.tp_frame_nr /= req.tp_frame_size;

	sock = SAFE_SOCKET(AF_PACKET, SOCK_RAW, 0);
	TEST(setsockopt(sock, SOL_PACKET, PACKET_VERSION, &version,
		sizeof(version)));

	if (TST_RET == -1 && TST_ERR == EINVAL)
		tst_brk(TCONF | TTERRNO, "TPACKET_V3 not supported");

	if (TST_RET) {
		tst_brk(TBROK | TTERRNO,
			"setsockopt(PACKET_VERSION, TPACKET_V3)");
	}

	/* Allocate owner map and then free it again */
	SAFE_SETSOCKOPT(sock, SOL_PACKET, PACKET_RX_RING, &req, sizeof(req));
	req.tp_block_nr = 0;
	req.tp_frame_nr = 0;
	SAFE_SETSOCKOPT(sock, SOL_PACKET, PACKET_RX_RING, &req, sizeof(req));

	/* Switch interface version and trigger double free of owner map */
	SAFE_SETSOCKOPT_INT(sock, SOL_PACKET, PACKET_VERSION, TPACKET_V2);
	SAFE_SETSOCKOPT(sock, SOL_PACKET, PACKET_RX_RING, &req, sizeof(req));
	SAFE_CLOSE(sock);

	tst_res(TPASS, "Nothing bad happened, probably");
}

static void cleanup(void)
{
	if (sock >= 0)
		SAFE_CLOSE(sock);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.timeout = 5,
	.taint_check = TST_TAINT_W | TST_TAINT_D,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_USER_NS=y",
		"CONFIG_NET_NS=y",
		NULL
	},
	.save_restore = (const struct tst_path_val const[]) {
		{"?/proc/sys/user/max_user_namespaces", NULL},
		NULL,
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "ec6af094ea28"},
		{"CVE", "2021-22600"},
		{}
	}
};
