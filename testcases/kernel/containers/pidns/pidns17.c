// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2007
 *               13/11/08  Gowrishankar M <gowrishankar.m@in.ibm.com>
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Clone a process with CLONE_NEWPID flag and spawn many children inside the
 * container. Then terminate all children and check if they were signaled.
 */

#include <sys/wait.h>
#include "tst_test.h"
#include "lapi/sched.h"

#define CHILDREN_NUM 10

static void child_func(void)
{
	int children[CHILDREN_NUM], status;
	unsigned int i;
	pid_t cpid, ppid;

	cpid = getpid();
	ppid = getppid();

	TST_EXP_EQ_LI(cpid, 1);
	TST_EXP_EQ_LI(ppid, 0);

	tst_res(TINFO, "Spawning %d children", CHILDREN_NUM);

	for (i = 0; i < CHILDREN_NUM; i++) {
		children[i] = SAFE_FORK();
		if (!children[i]) {
			pause();
			return;
		}
	}

	tst_res(TINFO, "Terminate children with SIGUSR1");

	SAFE_KILL(-1, SIGUSR1);

	for (i = 0; i < CHILDREN_NUM; i++) {
		SAFE_WAITPID(children[i], &status, 0);

		TST_EXP_EQ_LI(WIFSIGNALED(status), 1);
		TST_EXP_EQ_LI(WTERMSIG(status), SIGUSR1);
	}
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
};
