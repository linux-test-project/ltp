// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that :man2:`ioctl` returns ESRCH when a process attempts to access the
 * exit status of an isolated child using PIDFD_GET_INFO and PIDFD_INFO_EXIT
 * is not defined in struct pidfd_info.
 */

#include "ioctl_pidfd.h"
#include "lapi/sched.h"

static struct tst_clone_args *args;
static struct pidfd_info *info;

static void run(void)
{
	int status;
	int pidfd = 0;
	pid_t pid_child;

	memset(args, 0, sizeof(struct tst_clone_args));

	args->flags = CLONE_PIDFD | CLONE_NEWUSER | CLONE_NEWPID;
	args->pidfd = (uint64_t)&pidfd;
	args->exit_signal = SIGCHLD;

	pid_child = SAFE_CLONE(args);
	if (!pid_child)
		exit(100);

	info->mask = 0;

	/* child is not reaped, so ioctl() will pass */
	SAFE_IOCTL(pidfd, PIDFD_GET_INFO, info);
	TST_EXP_EQ_LI(info->mask & PIDFD_INFO_EXIT, 0);

	SAFE_WAITPID(pid_child, &status, 0);

	/* child is now reaped, so we get ESRCH */
	TST_EXP_FAIL(ioctl(pidfd, PIDFD_GET_INFO, info), ESRCH);
	TST_EXP_EQ_LI(info->mask & PIDFD_INFO_EXIT, 0);

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
