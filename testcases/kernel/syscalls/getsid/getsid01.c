// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2001
 *		07/2001 Ported by Wayne Boyer
 *   Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * Verify that session IDs returned by getsid() (with argument pid=0)
 * are same in parent and child process.
 */


#include "tst_test.h"

static pid_t p_sid;

static void run(void)
{
	pid_t pid, c_sid;

	TEST(getsid(0));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "getsid(0) failed in parent");
		return;
	}

	p_sid = TST_RET;

	pid = SAFE_FORK();

	if (pid == 0) {
		TEST(getsid(0));
		if (TST_RET == -1) {
			tst_res(TFAIL | TTERRNO, "getsid(0) failed in child");
			return;
		}
		c_sid = TST_RET;
		TST_EXP_EQ_LI(p_sid, c_sid);
	} else {
		SAFE_WAITPID(pid, NULL, 0);
	}
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1
};
