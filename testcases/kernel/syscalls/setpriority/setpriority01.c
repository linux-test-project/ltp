/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  03/2001 Written by Wayne Boyer
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
 * Verify that setpriority(2) succeeds set the scheduling priority of
 * the current process, process group or user.
 */

#include <errno.h>
#include <sys/resource.h>
#include <sys/time.h>
#include "tst_test.h"

static struct tcase {
	int which;
} tcases[] = {
	{PRIO_PROCESS},
	{PRIO_PGRP},
	{PRIO_USER}
};

static void verify_setpriority(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int new_prio, cur_prio;
	int failflag = 0;

	for (new_prio = -20; new_prio < 20; new_prio++) {
		TEST(setpriority(tc->which, 0, new_prio));

		if (TEST_RETURN != 0) {
			tst_res(TFAIL | TTERRNO,
				"setpriority(%d, 0, %d) failed",
				tc->which, new_prio);
			failflag = 1;
			continue;
		}

		cur_prio = SAFE_GETPRIORITY(tc->which, 0);

		if (cur_prio != new_prio) {
			tst_res(TFAIL, "current priority(%d) and "
				"new priority(%d) do not match",
				cur_prio, new_prio);
			failflag = 1;
		}
	}

	if (!failflag) {
		tst_res(TPASS, "setpriority(%d, 0, -20..19) succeeded",
			tc->which);
	}
}

static struct tst_test test = {
	.tid = "setpriority01",
	.tcnt = ARRAY_SIZE(tcases),
	.needs_root = 1,
	.test = verify_setpriority,
};
