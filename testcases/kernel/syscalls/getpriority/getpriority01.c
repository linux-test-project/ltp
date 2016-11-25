/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 *  11/2016 Modified by Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
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

	if (TEST_ERRNO != 0) {
		tst_res(TFAIL | TTERRNO, "getpriority(%d, 0) failed",
			tc->which);
		return;
	}

	if (TEST_RETURN < tc->min || TEST_RETURN > tc->max) {
		tst_res(TFAIL, "getpriority(%d, 0) returned %ld, "
			"expected in the range of [%d, %d]",
			tc->which, TEST_RETURN, tc->min, tc->max);
		return;
	}

	tst_res(TPASS, "getpriority(%d, 0) returned %ld",
		tc->which, TEST_RETURN);
}

static struct tst_test test = {
	.tid = "getpriority01",
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_getpriority,
};
