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
 * Verify that,
 *  1) setpriority(2) fails with -1 and sets errno to EINVAL if 'which'
 *     argument was not one of PRIO_PROCESS, PRIO_PGRP, or PRIO_USER.
 *  2) setpriority(2) fails with -1 and sets errno to ESRCH if no
 *     process was located for 'which' and 'who' arguments.
 *  3) setpriority(2) fails with -1 and sets errno to EACCES if an
 *     unprivileged user attempted to lower a process priority.
 *  4) setpriority(2) fails with -1 and sets errno to EPERM if an
 *     unprivileged user attempted to change a process which ID is
 *     different from the test process.
 */

#include <errno.h>
#include <pwd.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include "tst_test.h"

#define NEW_PRIO	-2
#define INVAL_FLAG	-1
#define INVAL_ID	-1
#define INIT_PID	1

static uid_t uid;

static struct tcase {
	int which;
	int who;
	int prio;
	int exp_errno;
	int unprivil;
} tcases[] = {
	{INVAL_FLAG, 0, NEW_PRIO, EINVAL, 0},

	{PRIO_PROCESS, INVAL_ID, NEW_PRIO, ESRCH, 0},
	{PRIO_PGRP, INVAL_ID, NEW_PRIO, ESRCH, 0},
	{PRIO_USER, INVAL_ID, NEW_PRIO, ESRCH, 0},

	{PRIO_PROCESS, 0, NEW_PRIO, EACCES, 1},
	{PRIO_PGRP, 0, NEW_PRIO, EACCES, 1},

	{PRIO_PROCESS, INIT_PID, NEW_PRIO, EPERM, 1}
};

static void setpriority_test(struct tcase *tc)
{
	char *desc = "";

	if (tc->unprivil)
		desc = "as unprivileged user ";

	TEST(setpriority(tc->which, tc->who, tc->prio));

	if (TEST_RETURN != -1) {
		tst_res(TFAIL,
			"setpriority(%d, %d, %d) %ssucceeds unexpectedly "
			"returned %ld", tc->which, tc->who, tc->prio, desc,
			TEST_RETURN);
		return;
	}

	if (TEST_ERRNO != tc->exp_errno) {
		tst_res(TFAIL | TTERRNO,
			"setpriority(%d, %d, %d) %sshould fail with %s",
			tc->which, tc->who, tc->prio, desc,
			tst_strerrno(tc->exp_errno));
		return;
	}

	tst_res(TPASS | TTERRNO,
		"setpriority(%d, %d, %d) %sfails as expected",
		tc->which, tc->who, tc->prio, desc);
}

static void verify_setpriority(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	if (tc->unprivil) {
		if (!SAFE_FORK()) {
			SAFE_SETUID(uid);
			SAFE_SETPGID(0, 0);
			setpriority_test(tc);
			exit(0);
		}

		tst_reap_children();
	} else {
		setpriority_test(tc);
	}
}

static void setup(void)
{
	struct passwd *ltpuser;

	ltpuser = SAFE_GETPWNAM("nobody");
	uid = ltpuser->pw_uid;
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.needs_root = 1,
	.forks_child = 1,
	.setup = setup,
	.test = verify_setpriority,
};
