// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2007
 *               27/12/07  Rishikesh K Rajak <risrajak@in.ibm.com>
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Clone a process with CLONE_NEWPID flag and check:
 *
 * - child process ID must be 1
 * - parent process ID must be 0
 */

#include "tst_test.h"
#include "lapi/sched.h"

static void child_func(void)
{
	pid_t cpid, ppid;

	cpid = tst_getpid();
	ppid = getppid();

	TST_EXP_EQ_LI(cpid, 1);
	TST_EXP_EQ_LI(ppid, 0);
}

static void run(void)
{
	const struct tst_clone_args args = { CLONE_NEWPID, SIGCHLD };

	if (!SAFE_CLONE(&args)) {
		child_func();
		return;
	}
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.forks_child = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_PID_NS",
		NULL,
	},
};
