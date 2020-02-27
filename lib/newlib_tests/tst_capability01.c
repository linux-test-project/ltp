/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2019 Richard Palethorpe <rpalethorpe@suse.com>
 *
 * The user or file requires CAP_NET_RAW for this test to work.
 * e.g use "$ setcap cap_net_raw=pei tst_capability"
 */

#include <unistd.h>
#include <sys/types.h>

#include "tst_test.h"
#include "tst_capability.h"
#include "tst_safe_net.h"

#include "lapi/socket.h"

static void run(void)
{
	TEST(socket(AF_INET, SOCK_RAW, 1));
	if (TST_RET > -1) {
		tst_res(TFAIL, "Created raw socket");
		SAFE_CLOSE(TST_RET);
	} else if (TST_ERR != EPERM) {
		tst_res(TFAIL | TTERRNO,
			"Failed to create socket for wrong reason");
	} else {
		tst_res(TPASS | TTERRNO, "Didn't create raw socket");
	}
}

static void setup(void)
{
	if (geteuid() == 0)
		tst_res(TWARN, "CAP_NET_RAW may be ignored when euid == 0");

	TEST(socket(AF_INET, SOCK_RAW, 1));
	if (TST_RET < 0)
		tst_brk(TFAIL | TTERRNO, "Can't create raw socket in setup");

	SAFE_CLOSE(TST_RET);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.caps = (struct tst_cap []) {
		TST_CAP(TST_CAP_REQ, CAP_NET_RAW),
		TST_CAP(TST_CAP_DROP, CAP_NET_RAW),
		{}
	},
};
