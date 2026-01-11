// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Check if the :manpage:`ioctl(2)` function allows retrieval of a child's exit code
 * using PIDFD_INFO_EXIT from a process that can be isolated or not from the
 * child.
 */

#include "ioctl_pidfd.h"
#include "lapi/sched.h"

static struct tst_clone_args *args;
static struct pidfd_info *info0, *info1;

static void run(unsigned int isolate)
{
	int status;
	int pidfd = 0;
	pid_t pid_child;

	memset(args, 0, sizeof(struct tst_clone_args));
	memset(info0, 0, sizeof(struct pidfd_info));
	memset(info1, 0, sizeof(struct pidfd_info));

	if (isolate) {
		args->flags = CLONE_PIDFD | CLONE_NEWUSER | CLONE_NEWPID;
		args->pidfd = (uint64_t)&pidfd;
		args->exit_signal = SIGCHLD;

		pid_child = SAFE_CLONE(args);
	} else {
		pid_child = SAFE_FORK();
	}

	if (!pid_child) {
		TST_CHECKPOINT_WAIT(0);
		exit(100);
	}

	if (!isolate)
		pidfd = SAFE_PIDFD_OPEN(pid_child, 0);

	/* child is not reaped and ioctl() won't provide any exit status info */
	info0->mask = PIDFD_INFO_EXIT;
	SAFE_IOCTL(pidfd, PIDFD_GET_INFO, info0);
	TST_EXP_EQ_LI(info0->mask & PIDFD_INFO_EXIT, 0);
	TST_EXP_EQ_LI(info0->exit_code, 0);

	TST_CHECKPOINT_WAKE(0);

	SAFE_WAITPID(pid_child, &status, 0);

	/* child is now reaped and ioctl() will provide the exit status */
	info1->mask = PIDFD_INFO_EXIT;
	SAFE_IOCTL(pidfd, PIDFD_GET_INFO, info1);
	SAFE_CLOSE(pidfd);

	TST_EXP_EQ_LI(info1->mask & PIDFD_INFO_EXIT, PIDFD_INFO_EXIT);
	TST_EXP_EQ_LI(info1->exit_code, status);

	TST_EXP_EQ_LI(WEXITSTATUS(info1->exit_code), 100);
}

static void setup(void)
{
	if (!ioctl_pidfd_info_exit_supported())
		tst_brk(TCONF, "PIDFD_INFO_EXIT is not supported by ioctl()");
}

static struct tst_test test = {
	.test = run,
	.setup = setup,
	.tcnt = 2,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.bufs = (struct tst_buffers []) {
		{&args, .size = sizeof(*args)},
		{&info0, .size = sizeof(*info0)},
		{&info1, .size = sizeof(*info1)},
		{}
	}
};
