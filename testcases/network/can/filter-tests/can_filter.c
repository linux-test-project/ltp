// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2011 Volkswagen Group Electronic Research
 * Copyright (c) 2021 SUSE LLC
 */

#include "config.h"
#include "tst_test.h"

#ifdef HAVE_LINUX_CAN_H

#include "can_common.h"

#define ID 0x123
#define TC 18			/* # of testcases */

const int rx_res[TC] = { 4, 4, 4, 4, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1 };
const int rxbits_res[TC] = { 4369, 4369, 4369, 4369, 17, 4352, 17, 4352, 257,
			     257, 4112, 4112, 1, 256, 16, 4096, 1, 256 };

static int s;

static canid_t calc_id(int testcase)
{
	canid_t id = ID;

	if (testcase & 1)
		id |= CAN_EFF_FLAG;
	if (testcase & 2)
		id |= CAN_RTR_FLAG;

	return id;
}

static canid_t calc_mask(int testcase)
{
	canid_t mask = CAN_SFF_MASK;

	if (testcase > 15)
		return CAN_EFF_MASK | CAN_EFF_FLAG | CAN_RTR_FLAG;

	if (testcase & 4)
		mask |= CAN_EFF_FLAG;
	if (testcase & 8)
		mask |= CAN_RTR_FLAG;

	return mask;
}

static void setup(void)
{
	struct sockaddr_can addr;
	struct ifreq ifr;
	int recv_own_msgs = 1;

	can_setup_vcan();

	s = SAFE_SOCKET(PF_CAN, SOCK_RAW, CAN_RAW);

	strcpy(ifr.ifr_name, can_dev_name);
	SAFE_IOCTL(s, SIOCGIFINDEX, &ifr);

	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	SAFE_SETSOCKOPT(s, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS,
			&recv_own_msgs, sizeof(recv_own_msgs));

	SAFE_BIND(s, (struct sockaddr *)&addr, sizeof(addr));
}

static void cleanup(void)
{
	if (s)
		SAFE_CLOSE(s);

	can_cleanup_vcan();
}

static void run(unsigned int n)
{
	fd_set rdfs;
	struct timeval tv;
	struct can_frame frame;
	static struct can_filter rfilter;
	int testcase = n;
	int have_rx = 1;
	int rx = 0;
	int rxbits = 0, rxbitval;

	rfilter.can_id = calc_id(testcase);
	rfilter.can_mask = calc_mask(testcase);
	SAFE_SETSOCKOPT(s, SOL_CAN_RAW, CAN_RAW_FILTER,
			&rfilter, sizeof(rfilter));

	tst_res(TINFO, "testcase %2d filters : can_id = 0x%08X can_mask = "
	       "0x%08X", testcase, rfilter.can_id, rfilter.can_mask);

	tst_res(TINFO, "testcase %2d sending patterns ... ", testcase);

	frame.can_dlc = 1;
	frame.data[0] = testcase;

	frame.can_id = ID;
	SAFE_WRITE(SAFE_WRITE_ALL, s, &frame, sizeof(frame));

	frame.can_id = (ID | CAN_RTR_FLAG);
	SAFE_WRITE(SAFE_WRITE_ALL, s, &frame, sizeof(frame));

	frame.can_id = (ID | CAN_EFF_FLAG);
	SAFE_WRITE(SAFE_WRITE_ALL, s, &frame, sizeof(frame));

	frame.can_id = (ID | CAN_EFF_FLAG | CAN_RTR_FLAG);
	SAFE_WRITE(SAFE_WRITE_ALL, s, &frame, sizeof(frame));

	tst_res(TPASS, "testcase %2d Sent patterns", testcase);

	while (have_rx) {

		have_rx = 0;
		FD_ZERO(&rdfs);
		FD_SET(s, &rdfs);
		tv.tv_sec = 0;
		tv.tv_usec = 50000;	/* 50ms timeout */

		if (select(s + 1, &rdfs, NULL, NULL, &tv) < 0)
			tst_brk(TBROK | TERRNO, "select");

		if (FD_ISSET(s, &rdfs)) {
			have_rx = 1;
			SAFE_READ(1, s, &frame, sizeof(struct can_frame));

			if ((frame.can_id & CAN_SFF_MASK) != ID)
				tst_res(TFAIL, "received wrong can_id!");

			if (frame.data[0] != testcase)
				tst_res(TFAIL, "received wrong testcase!");

			/* test & calc rxbits */
			rxbitval = 1 << ((frame.can_id &
					  (CAN_EFF_FLAG | CAN_RTR_FLAG |
					   CAN_ERR_FLAG)) >> 28);

			/* only receive a rxbitval once */
			if ((rxbits & rxbitval) == rxbitval) {
				tst_res(TFAIL, "received rxbitval %d twice!",
					rxbitval);
			}
			rxbits |= rxbitval;
			rx++;

			tst_res(TINFO, "testcase %2d rx : can_id = 0x%08X rx = "
			       "%d rxbits = %d", testcase,
			       frame.can_id, rx, rxbits);
		}
	}
	/* rx timed out -> check the received results */
	if (rx_res[testcase] != rx) {
		tst_brk(TBROK,
			"wrong rx value in testcase %d : %d (expected "
			"%d)", testcase, rx, rx_res[testcase]);
	}
	if (rxbits_res[testcase] != rxbits) {
		tst_brk(TBROK,
			"wrong rxbits value in testcase %d : %d "
			"(expected %d)", testcase, rxbits,
			rxbits_res[testcase]);
	}
	tst_res(TPASS, "testcase %2d ok", testcase);
}

static struct tst_test test = {
	.tcnt = TC,
	.options = (struct tst_option[]) {
		{"d:", &can_dev_name, "CAN device name"},
		{}
	},
	.setup = setup,
	.cleanup = cleanup,
	.test = run,
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
