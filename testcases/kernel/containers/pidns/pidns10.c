// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) International Business Machines Corp., 2008
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Clone a process with CLONE_NEWPID flag and check that killing subprocesses
 * from child namespace will raise ESRCH error.
 */

#include "tst_test.h"
#include "lapi/sched.h"

static void child_func(void)
{
	pid_t cpid = tst_getpid();
	pid_t ppid = getppid();

	TST_EXP_EQ_LI(cpid, 1);
	TST_EXP_EQ_LI(ppid, 0);

	tst_res(TINFO, "Trying to kill all subprocesses from within container");

	TST_EXP_FAIL(kill(-1, SIGKILL), ESRCH);
}

static void run(void)
{
	const struct tst_clone_args args = {
		.flags = CLONE_NEWPID,
		.exit_signal = SIGCHLD,
	};

	if (!SAFE_CLONE(&args)) {
		child_func();
		return;
	}
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.forks_child = 1,
};
