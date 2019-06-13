// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Ported to LTP: Wayne Boyer
 *  11/2016 Modified by Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 */

/*
 * Verify that getpriority(2) succeeds get the scheduling priority of
 * the current process, process group or user, and the priority values
 * are in the ranges of [0, 0], [0, 0] and [-20, 0] by default for the
 * flags PRIO_PROCESS, PRIO_PGRP and PRIO_USER respectively.
 */

#include <errno.h>
#include <sys/resource.h>
#include <sys/time.h>
#include "tst_test.h"

static struct tcase {
	int which;
	int min;
	int max;
} tcases[] = {
	{PRIO_PROCESS, 0, 0},
	{PRIO_PGRP, 0, 0},
	{PRIO_USER, -20, 0}
};

static void verify_getpriority(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(getpriority(tc->which, 0));

	if (TST_ERR != 0) {
		tst_res(TFAIL | TTERRNO, "getpriority(%d, 0) failed",
			tc->which);
		return;
	}

	if (TST_RET < tc->min || TST_RET > tc->max) {
		tst_res(TFAIL, "getpriority(%d, 0) returned %ld, "
			"expected in the range of [%d, %d]",
			tc->which, TST_RET, tc->min, tc->max);
		return;
	}

	tst_res(TPASS, "getpriority(%d, 0) returned %ld",
		tc->which, TST_RET);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_getpriority,
};
