// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) International Business Machines Corp., 2008
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Clone a process with CLONE_NEWPID flag and check that parent process can't
 * be killed from child namespace.
 */

#include "tst_test.h"
#include "lapi/sched.h"

static void child_func(int pid)
{
	pid_t cpid = tst_getpid();
	pid_t ppid = getppid();

	TST_EXP_EQ_LI(cpid, 1);
	TST_EXP_EQ_LI(ppid, 0);

	tst_res(TINFO, "Trying to kill parent from within container");

	TST_EXP_FAIL(kill(pid, SIGKILL), ESRCH);
}

static void run(void)
{
	const struct tst_clone_args args = {
		.flags = CLONE_NEWPID,
		.exit_signal = SIGCHLD,
	};
	pid_t pid = getpid();

	if (!SAFE_CLONE(&args)) {
		child_func(pid);
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
