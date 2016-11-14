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
 * Verify that,
 *  1) getpriority(2) fails with -1 and sets errno to EINVAL if 'which'
 *     argument was not one of PRIO_PROCESS, PRIO_PGRP, or PRIO_USER.
 *  2) getpriority(2) fails with -1 and sets errno to ESRCH if no
 *     process was located for 'which' and 'who' arguments.
 */

#include <errno.h>
#include <sys/resource.h>
#include <sys/time.h>
#include "tst_test.h"

#define INVAL_FLAG	-1
#define INVAL_ID	-1

static struct tcase {
	int which;
	int who;
	int exp_errno;
} tcases[] = {
	/* test 1 */
	{INVAL_FLAG, 0, EINVAL},

	/* test 2 */
	{PRIO_PROCESS, INVAL_ID, ESRCH},
	{PRIO_PGRP, INVAL_ID, ESRCH},
	{PRIO_USER, INVAL_ID, ESRCH}
};

static void verify_getpriority(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(getpriority(tc->which, tc->who));

	if (TEST_RETURN != -1) {
		tst_res(TFAIL, "getpriority(%d, %d) succeeds unexpectedly, "
			       "returned %li", tc->which, tc->who, TEST_RETURN);
		return;
	}

	if (tc->exp_errno != TEST_ERRNO) {
		tst_res(TFAIL | TTERRNO,
			"getpriority(%d, %d) should fail with %s",
			tc->which, tc->who, tst_strerrno(tc->exp_errno));
		return;
	}

	tst_res(TPASS | TTERRNO, "getpriority(%d, %d) fails as expected",
		tc->which, tc->who);
}

static struct tst_test test = {
	.tid = "getpriority02",
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_getpriority,
};
