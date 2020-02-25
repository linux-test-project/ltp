// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Description:
 * Basic pidfd_open() test to test invalid arguments.
 */
#include "tst_test.h"
#include "lapi/pidfd_open.h"

pid_t expired_pid, my_pid, invalid_pid = -1;

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
	expired_pid = tst_get_unused_pid();
	my_pid = getpid();
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(pidfd_open(*tc->pid, tc->flags));

	if (TST_RET != -1) {
		SAFE_CLOSE(TST_RET);
		tst_res(TFAIL, "%s: pidfd_open succeeded unexpectedly (index: %d)",
			tc->name, n);
		return;
	}

	if (tc->exp_errno != TST_ERR) {
		tst_res(TFAIL | TTERRNO, "%s: pidfd_open() should fail with %s",
			tc->name, tst_strerrno(tc->exp_errno));
		return;
	}

	tst_res(TPASS | TTERRNO, "%s: pidfd_open() failed as expected",
		tc->name);
}

static struct tst_test test = {
	.min_kver = "5.3",
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = setup,
};
