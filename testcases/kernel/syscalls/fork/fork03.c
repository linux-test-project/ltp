// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Copyright (c) Linux Test Project, 2003-2023
 * Author: 2001 Ported by Wayne Boyer
 */

/*\
 * Check that child process can use a large text space and do a large number
 * of operations. In this situation, check for pid == 0 in child and check
 * for pid > 0 in parent after wait.
 */

#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "tst_test.h"

static void verify_fork(void)
{
	float fl1, fl2;
	int pid1, pid2, status, i;

	pid1 = SAFE_FORK();
	if (!pid1) {
		/* child uses some cpu time slices */
		for (i = 1; i < 32767; i++) {
			fl1 = 0.000001;
			fl1 = fl2 = 0.000001;
			fl1 = fl1 * 10.0;
			fl2 = fl1 / 1.232323;
			fl1 = fl2 - fl2;
			fl1 = fl2;
		}
		exit(!!pid1);
	}

	tst_res(TINFO, "process id in parent of child from fork: %d", pid1);
	pid2 = SAFE_WAIT(&status);

	if (pid1 != pid2) {
		tst_res(TFAIL, "pids don't match: %d vs %d", pid1, pid2);
		return;
	}

	if ((status >> 8) != 0) {
		tst_res(TFAIL, "child exited with failure");
		return;
	}

	tst_res(TPASS, "test PASSED");
}

static struct tst_test test = {
	.test_all = verify_fork,
	.forks_child = 1,
};
