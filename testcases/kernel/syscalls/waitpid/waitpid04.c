// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2024 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * Test to check the error conditions in waitpid() syscall.
 */

#include <signal.h>
#include <sys/wait.h>
#include "tst_test.h"

static struct testcase {
	pid_t pid;
	int flags;
	int err;
} testcase_list[] = {
	{-1, 0, ECHILD},	/* Wait for any child when none exist */
	{1, 0, ECHILD},		/* Wait for non-child process */
	{-1, -1, EINVAL},	/* Invalid flags */
	{INT_MIN, 0, ESRCH},	/* Wait for invalid process group */
};

static void run(unsigned int n)
{
	const struct testcase *tc = testcase_list + n;

	TST_EXP_FAIL2(waitpid(tc->pid, NULL, tc->flags), tc->err,
		"waipid(%d, NULL, 0x%x)", tc->pid, tc->flags);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(testcase_list)
};
