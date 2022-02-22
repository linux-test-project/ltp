// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 */

/*\
 * [Description]
 *
 * Tests basic error handling of the pidfd_open syscall.
 *
 * - ESRCH the process specified by pid does not exist
 * - EINVAL pid is not valid
 * - EINVAL flags is not valid
 */
#include "tst_test.h"
#include "lapi/pidfd.h"

static pid_t expired_pid, my_pid, invalid_pid = -1;

static struct tcase {
	char *name;
	pid_t *pid;
	int flags;
	int exp_errno;
} tcases[] = {
	{"expired pid", &expired_pid, 0, ESRCH},
	{"invalid pid", &invalid_pid, 0, EINVAL},
	{"invalid flags", &my_pid, 1, EINVAL},
};

static void setup(void)
{
	pidfd_open_supported();
	expired_pid = tst_get_unused_pid();
	my_pid = getpid();
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TST_EXP_FAIL2(pidfd_open(*tc->pid, tc->flags), tc->exp_errno,
			"pidfd_open with %s", tc->name);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = setup,
};
