// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2011 Volkswagen Group Electronic Research
 * Copyright (c) 2021 SUSE LLC
 */

#include "config.h"
#include "tst_test.h"

#ifdef HAVE_LINUX_CAN_H

#include "can_common.h"
#include "tst_minmax.h"

static int s, t;

static void test_sockets(canid_t can_id, int expect_rxs, int expect_rxt)
{
	fd_set rdfs;
	struct timeval tv;
	int m = MAX(s, t) + 1;
	int have_rx = 1;
	struct can_frame frame;
	int rxs = 0, rxt = 0;

	frame.can_id = can_id;
	frame.can_dlc = 0;
	SAFE_WRITE(SAFE_WRITE_ALL, s, &frame, sizeof(frame));

	while (have_rx) {

		FD_ZERO(&rdfs);
		FD_SET(s, &rdfs);
		FD_SET(t, &rdfs);
		tv.tv_sec = 0;
		tv.tv_usec = 50000;	/* 50ms timeout */
		have_rx = 0;

		if (select(m, &rdfs, NULL, NULL, &tv) < 0)
			tst_brk(TBROK | TERRNO, "select");

		if (FD_ISSET(s, &rdfs)) {

			have_rx = 1;
			SAFE_READ(1, s, &frame, sizeof(struct can_frame));

			if (frame.can_id != can_id)
				tst_res(TFAIL, "received wrong can_id!");

			rxs++;
		}

		if (FD_ISSET(t, &rdfs)) {

			have_rx = 1;
			SAFE_READ(1, t, &frame, sizeof(struct can_frame));

			if (frame.can_id != can_id)
				tst_res(TFAIL, "received wrong can_id!");

			rxt++;
		}
	}

	/* timeout */

	tst_res(rxs == expect_rxs && rxt == expect_rxt ? TPASS : TFAIL,
		"s received %d of %d, t received %d of %d",
		rxs, expect_rxs, rxt, expect_rxt);
}

static void setopts(int loopback, int recv_own_msgs)
{
	SAFE_SETSOCKOPT(s, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback,
			sizeof(loopback));
	SAFE_SETSOCKOPT(s, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &recv_own_msgs,
			sizeof(recv_own_msgs));

	tst_res(TINFO, "set loopback = %d, recv_own_msgs = %d",
		loopback, recv_own_msgs);
}

static void setup(void)
{
	struct sockaddr_can addr;
	struct ifreq ifr;

	can_setup_vcan();

	s = SAFE_SOCKET(PF_CAN, SOCK_RAW, CAN_RAW);
	t = SAFE_SOCKET(PF_CAN, SOCK_RAW, CAN_RAW);

	strcpy(ifr.ifr_name, can_dev_name);
	SAFE_IOCTL(s, SIOCGIFINDEX, &ifr);

	addr.can_ifindex = ifr.ifr_ifindex;
	addr.can_family = AF_CAN;

	SAFE_BIND(s, (struct sockaddr *)&addr, sizeof(addr));
	SAFE_BIND(t, (struct sockaddr *)&addr, sizeof(addr));
}

static void cleanup(void)
{
	if (s)
		SAFE_CLOSE(s);
	if (t)
		SAFE_CLOSE(t);

	can_cleanup_vcan();
}

static void run(void)
{
	tst_res(TINFO, "Starting PF_CAN frame flow test.");
	tst_res(TINFO, "checking socket default settings");
	test_sockets(0x340, 0, 1);

	setopts(0, 0);
	test_sockets(0x341, 0, 0);

	setopts(0, 1);
	test_sockets(0x342, 0, 0);

	setopts(1, 0);
	test_sockets(0x343, 0, 1);

	setopts(1, 1);
	test_sockets(0x344, 1, 1);

	/* Return to defaults for when -i is used */
	setopts(1, 0);
}

static struct tst_test test = {
	.options = (struct tst_option[]) {
		{"d:", &can_dev_name, "CAN device name"},
		{}
	},
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
	.caps = (struct tst_cap []) {
		TST_CAP(TST_CAP_REQ, CAP_NET_RAW),
		TST_CAP(TST_CAP_DROP, CAP_SYS_ADMIN),
		{}
	},
	.needs_drivers = (const char *const[]) {
		"vcan",
		"can-raw",
		NULL
	}
};

#else

TST_TEST_TCONF("The linux/can.h was missing upon compilation");

#endif /* HAVE_LINUX_CAN_H */
