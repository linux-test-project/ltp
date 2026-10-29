// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that :manpage:`ioctl(2)` permits to obtain the exit code of an isolated
 * signaled child via PIDFD_INFO_EXIT from within a process.
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
	memset(info, 0, sizeof(struct pidfd_info));

	info->mask = PIDFD_INFO_EXIT;

	args->flags = CLONE_PIDFD | CLONE_NEWUSER | CLONE_NEWPID;
	args->pidfd = TST_PTR_TO_UINT(&pidfd);
	args->exit_signal = SIGCHLD;

	pid_child = SAFE_CLONE(args);
	if (!pid_child) {
		TST_CHECKPOINT_WAKE_AND_WAIT(0);
		exit(1);
	}

	TST_CHECKPOINT_WAIT(0);

	SAFE_KILL(pid_child, SIGKILL);
	SAFE_WAITPID(pid_child, &status, 0);

	SAFE_IOCTL(pidfd, PIDFD_GET_INFO, info);
	SAFE_CLOSE(pidfd);

	TST_EXP_EQ_LI(info->mask & PIDFD_INFO_EXIT, PIDFD_INFO_EXIT);
	TST_EXP_EQ_LI(WIFSIGNALED(info->exit_code), WIFSIGNALED(status));
	TST_EXP_EQ_LI(WEXITSTATUS(info->exit_code), WEXITSTATUS(status));
	TST_EXP_EQ_LI(WTERMSIG(info->exit_code), WTERMSIG(status));

	TST_EXP_EXPR(WIFSIGNALED(info->exit_code) &&
	      WTERMSIG(info->exit_code) == SIGKILL);
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
	.needs_checkpoints = 1,
	.bufs = (struct tst_buffers []) {
		{&args, .size = sizeof(*args)},
		{&info, .size = sizeof(*info)},
		{}
	},
	.needs_kconfigs = (const char *[]) {
		"CONFIG_USER_NS",
		"CONFIG_PID_NS",
		NULL
	}
};
