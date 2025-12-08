// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that :man2:`ioctl` doesn't allow to obtain the exit status of an isolated
 * process via PIDFD_INFO_EXIT in within an another isolated process, which
 * doesn't have any parent connection.
 */

#include "ioctl_pidfd.h"
#include "lapi/sched.h"

static struct tst_clone_args *args;
static struct pidfd_info *info;

static void run(void)
{
	int pidfd;
	pid_t pid_child;

	memset(args, 0, sizeof(struct tst_clone_args));
	memset(info, 0, sizeof(struct pidfd_info));

	info->mask = PIDFD_INFO_EXIT;

	args->flags = CLONE_PIDFD | CLONE_NEWUSER | CLONE_NEWPID;
	args->pidfd = (uint64_t)&pidfd;
	args->exit_signal = SIGCHLD;

	pid_child = SAFE_CLONE(args);
	if (!pid_child)
		exit(100);

	SAFE_WAITPID(pid_child, NULL, 0);

	memset(args, 0, sizeof(struct tst_clone_args));

	args->flags = CLONE_NEWUSER | CLONE_NEWPID;
	args->exit_signal = SIGCHLD;

	if (!SAFE_CLONE(args)) {
		TST_EXP_FAIL(ioctl(pidfd, PIDFD_GET_INFO, info), ESRCH);
		exit(0);
	}

	SAFE_CLOSE(pidfd);
}

static void setup(void)
{
	if (!ioctl_pidfd_info_exit_supported())
		tst_brk(TCONF, "PIDFD_INFO_EXIT is not supported by ioctl()");
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.forks_child = 1,
	.bufs = (struct tst_buffers []) {
		{&args, .size = sizeof(*args)},
		{&info, .size = sizeof(*info)},
		{}
	}
};
