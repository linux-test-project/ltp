// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Veerendra C <vechandr@in.ibm.com>, 2008
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Clone a process with CLONE_NEWPID flag and create many levels of child
 * containers. Then kill container init process from parent and check if all
 * containers have been killed.
 */

#include <sys/wait.h>
#include "tst_test.h"
#include "lapi/sched.h"

#define MAX_DEPTH	5

static struct tst_clone_args clone_args = {
	.flags = CLONE_NEWPID,
	.exit_signal = SIGCHLD
};
static pid_t pid_max;

static void child_func(int *level)
{
	pid_t cpid, ppid;

	cpid = tst_getpid();
	ppid = getppid();

	TST_EXP_EQ_LI(cpid, 1);
	TST_EXP_EQ_LI(ppid, 0);

	if (*level >= MAX_DEPTH) {
		TST_CHECKPOINT_WAKE(0);
		return;
	}

	(*level)++;

	if (!SAFE_CLONE(&clone_args)) {
		child_func(level);
		return;
	}

	pause();
}

static int find_cinit_pids(pid_t *pids)
{
	int pid;
	int next = 0;
	pid_t parentpid, pgid, pgid2;

	parentpid = tst_getpid();
	pgid = SAFE_GETPGID(parentpid);

	for (pid = 2; pid < pid_max; pid++) {
		if (pid == parentpid)
			continue;

		pgid2 = getpgid(pid);

		if (pgid2 == pgid) {
			pids[next] = pid;
			next++;
		}
	}

	return next;
}

static void setup(void)
{
	SAFE_FILE_SCANF("/proc/sys/kernel/pid_max", "%d\n", &pid_max);
}

static void run(void)
{
	int i, status, children;
	int level = 0;
	pid_t pids_new[MAX_DEPTH];
	pid_t pids[MAX_DEPTH];
	pid_t pid;

	pid = SAFE_CLONE(&clone_args);
	if (!pid) {
		child_func(&level);
		return;
	}

	TST_CHECKPOINT_WAIT(0);

	TST_EXP_POSITIVE(find_cinit_pids(pids));

	SAFE_KILL(pid, SIGKILL);
	SAFE_WAITPID(0, &status, 0);

	children = find_cinit_pids(pids_new);

	if (children > 0) {
		tst_res(TFAIL, "%d children left after sending SIGKILL", children);

		for (i = 0; i < MAX_DEPTH; i++) {
			kill(pids[i], SIGKILL);
			waitpid(pids[i], &status, 0);
		}

		return;
	}

	tst_res(TPASS, "No children left after sending SIGKILL to the first child");
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.needs_root = 1,
	.needs_checkpoints = 1,
	.forks_child = 1,
};
