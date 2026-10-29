// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 Ported by Wayne Boyer
 */

/*\
 * Verify that kill(2) fails with the correct error codes:
 *
 * - EINVAL if given an invalid signal.
 * - ESRCH if given a non-existent pid.
 * - ESRCH if the given pid is INT_MIN.
 */

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include "tst_test.h"

static pid_t real_pid, fake_pid, int_min_pid;

static struct tcase {
	int test_sig;
	int exp_errno;
	pid_t *pid;
} tcases[] = {
	{2000, EINVAL, &real_pid},
	{SIGKILL, ESRCH, &fake_pid},
	{SIGKILL, ESRCH, &int_min_pid}
};

static void verify_kill(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(kill(*tc->pid, tc->test_sig));
	if (TST_RET != -1) {
		tst_res(TFAIL, "kill should fail but not, return %ld", TST_RET);
		return;
	}

	if (tc->exp_errno == TST_ERR)
		tst_res(TPASS | TTERRNO, "kill failed as expected");
	else
		tst_res(TFAIL | TTERRNO, "kill expected %s but got",
			tst_strerrno(tc->exp_errno));
}

static void setup(void)
{
	real_pid = getpid();
	fake_pid = tst_get_unused_pid();
	int_min_pid = INT_MIN;
}

static struct tst_test test = {
	.test = verify_kill,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
};
