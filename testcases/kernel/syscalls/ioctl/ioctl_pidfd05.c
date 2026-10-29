// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that :manpage:`ioctl(2)` raises an EINVAL or ENOTTY (since v6.18-rc1) error
 * when PIDFD_GET_INFO is used. This happens when:
 *
 * - info parameter is NULL
 * - info parameter is providing the wrong size
 */

#include "tst_test.h"
#include "lapi/pidfd.h"
#include "lapi/sched.h"
#include <errno.h>
#include "ioctl_pidfd.h"

struct pidfd_info_invalid {
	uint32_t dummy;
};

#define PIDFD_GET_INFO_SHORT _IOWR(PIDFS_IOCTL_MAGIC, 11, struct pidfd_info_invalid)

static struct tst_clone_args *args;
static struct pidfd_info_invalid *info_invalid;

static void run(void)
{
	int pidfd = 0;
	pid_t pid_child;

	memset(args, 0, sizeof(struct tst_clone_args));

	info_invalid->dummy = 1;

	args->flags = CLONE_PIDFD | CLONE_NEWUSER | CLONE_NEWPID;
	args->pidfd = TST_PTR_TO_UINT(&pidfd);
	args->exit_signal = SIGCHLD;

	pid_child = SAFE_CLONE(args);
	if (!pid_child)
		exit(0);

	TST_EXP_FAIL(ioctl(pidfd, PIDFD_GET_INFO, NULL), EINVAL);

	/*
	 * Expect ioctl to fail; accept either EINVAL or ENOTTY (v6.18-rc1,
	 * 3c17001b21b9f ("pidfs: validate extensible ioctls")).
	 */
	int exp_errnos[] = {EINVAL, ENOTTY};

	TST_EXP_FAIL_ARR(ioctl(pidfd, PIDFD_GET_INFO_SHORT, info_invalid),
			exp_errnos, ARRAY_SIZE(exp_errnos));

	SAFE_CLOSE(pidfd);
}

static void setup(void)
{
	if (!ioctl_pidfd_get_info_supported())
		tst_brk(TCONF, "ioctl(PIDFD_GET_INFO) is not implemented");
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.forks_child = 1,
	.bufs = (struct tst_buffers []) {
		{&args, .size = sizeof(*args)},
		{&info_invalid, .size = sizeof(*info_invalid)},
		{}
	},
	.needs_kconfigs = (const char *[]) {
		"CONFIG_USER_NS",
		"CONFIG_PID_NS",
		NULL
	}
};
