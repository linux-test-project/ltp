// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2020 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * CVE-2021-3609
 *
 * Test for race condition vulnerability in CAN BCM. Fixed in:
 * d5f9023fa61e ("can: bcm: delay release of struct bcm_op after synchronize_rcu()").
 *
 * The test is skipped when running in 32-bit compat mode. The kernel
 * compatibility layer for CAN structures is not implemented at the
 * time of writing.
 */

#include "config.h"
#include "tst_test.h"

#ifdef HAVE_LINUX_CAN_H

#include <linux/can.h>
#include <linux/can/bcm.h>

#include "tst_netdevice.h"
#include "tst_fuzzy_sync.h"

#define LTP_DEVICE "ltp_vcan0"

struct test_payload {
	struct bcm_msg_head head;
	struct can_frame frame;
};

static int sock1 = -1, sock2 = -1;
static struct tst_fzsync_pair fzsync_pair;

static void setup(void)
{
	struct sockaddr_can addr = { .can_family = AF_CAN };

	/*
	 * Older kernels require explicit modprobe of vcan. Newer kernels
	 * will load the modules automatically and support CAN in network
	 * namespace which would eliminate the need for running the test
	 * with root privileges.
	 */
	tst_cmd((const char*[]){"modprobe", "vcan", NULL}, NULL, NULL, 0);

	NETDEV_ADD_DEVICE(LTP_DEVICE, "vcan");
	NETDEV_SET_STATE(LTP_DEVICE, 1);
	addr.can_ifindex = NETDEV_INDEX_BY_NAME(LTP_DEVICE);
	addr.can_addr.tp.rx_id = 1;
	sock1 = SAFE_SOCKET(AF_CAN, SOCK_DGRAM, CAN_BCM);
	SAFE_CONNECT(sock1, (struct sockaddr *)&addr, sizeof(addr));

	fzsync_pair.exec_loops = 100000;
	tst_fzsync_pair_init(&fzsync_pair);
}

static void *thread_run(void *arg)
{
	struct test_payload data = {
		{
			.opcode = TX_SEND,
			.flags = RX_NO_AUTOTIMER,
			.count = -1,
			.nframes = 1
		},
		{0}
	};
	struct iovec iov = {
		.iov_base = &data,
		.iov_len = sizeof(data)
	};
	struct msghdr msg = {
		.msg_iov = &iov,
		.msg_iovlen = 1
	};

	while (tst_fzsync_run_b(&fzsync_pair)) {
		tst_fzsync_start_race_b(&fzsync_pair);
		SAFE_SENDMSG(iov.iov_len, sock1, &msg, 0);
		tst_fzsync_end_race_b(&fzsync_pair);
	}

	return arg;
}

static void run(void)
{
	struct sockaddr_can addr = { .can_family = AF_CAN };
	struct bcm_msg_head data = {
		.opcode = RX_SETUP,
		.flags = RX_FILTER_ID | SETTIMER | STARTTIMER,
		.ival1.tv_sec = 1,
		.ival2.tv_sec = 1
	};
	struct iovec iov = {
		.iov_base = &data,
		.iov_len = sizeof(data)
	};
	struct msghdr msg = {
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};

	tst_fzsync_pair_reset(&fzsync_pair, thread_run);

	while (tst_fzsync_run_a(&fzsync_pair)) {
		sock2 = SAFE_SOCKET(AF_CAN, SOCK_DGRAM, CAN_BCM);
		SAFE_CONNECT(sock2, (struct sockaddr *)&addr, sizeof(addr));
		SAFE_SENDMSG(iov.iov_len, sock2, &msg, 0);
		tst_fzsync_start_race_a(&fzsync_pair);
		SAFE_CLOSE(sock2);
		tst_fzsync_end_race_a(&fzsync_pair);
	}

	tst_res(TPASS, "Nothing bad happened, probably");
}

static void cleanup(void)
{
	tst_fzsync_pair_cleanup(&fzsync_pair);

	if (sock1 >= 0)
		SAFE_CLOSE(sock1);

	if (sock2 >= 0)
		SAFE_CLOSE(sock2);

	NETDEV_REMOVE_DEVICE(LTP_DEVICE);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.taint_check = TST_TAINT_W | TST_TAINT_D,
	.needs_root = 1,
	.skip_in_compat = 1,
	.min_runtime = 30,
	.needs_drivers = (const char *const[]) {
		"vcan",
		"can-bcm",
		NULL
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "d5f9023fa61e"},
		{"CVE", "2021-3609"},
		{}
	}
};

#else

TST_TEST_TCONF("The test was built without <linux/can.h>");

#endif /* HAVE_LINUX_CAN_H */
