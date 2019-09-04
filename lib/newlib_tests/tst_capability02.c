/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2019 Richard Palethorpe <rpalethorpe@suse.com>
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
		tst_res(TPASS, "Created raw socket");
		SAFE_CLOSE(TST_RET);
	} else {
		tst_res(TFAIL | TTERRNO, "Didn't create raw socket");
	}
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.caps = (struct tst_cap []) {
		TST_CAP(TST_CAP_REQ, CAP_NET_RAW),
		TST_CAP(TST_CAP_DROP, CAP_AUDIT_READ), /* 64bit capability */
		TST_CAP(TST_CAP_DROP, CAP_SYS_ADMIN),
		{}
	},
};
