// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) International Business Machines Corp., 2008
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Clone a process with CLONE_NEWPID flag and check that child container does
 * not get kill itself with SIGKILL.
 */

#include <sys/wait.h>
#include "tst_test.h"
#include "lapi/sched.h"

static void child_func(void)
{
	pid_t cpid = getpid();
	pid_t ppid = getppid();

	TST_EXP_EQ_LI(cpid, 1);
	TST_EXP_EQ_LI(ppid, 0);

	tst_res(TINFO, "Trying to kill container from within container");

	SAFE_KILL(1, SIGKILL);

	tst_res(TINFO, "Container is up and running");
}

static void run(void)
{
	const struct tst_clone_args args = { CLONE_NEWPID, SIGCHLD };
	pid_t pid;

	pid = SAFE_CLONE(&args);
	if (!pid) {
		child_func();
		return;
	}

	tst_reap_children();
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.forks_child = 1,
};
