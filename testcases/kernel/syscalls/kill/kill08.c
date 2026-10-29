// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) IBM  Corp., 07/2001 Ported by Wayne Boyer
 * Copyright (c) 2025 SUSE LLC Ricardo B. Marlière <rbm@suse.com>
 */

/*\
 * Test checks the basic functionality of kill() when killing an
 * entire process group.
 */

#include "tst_test.h"

#define TEST_SIG SIGKILL

static void run(void)
{
	pid_t pid;
	int status, nsig, i;

	/*
	 * Fork a process and set the process group so that
	 * it is different from this one.  Fork 5 more children.
	 */
	pid = SAFE_FORK();
	if (!pid) {
		setpgrp();
		for (i = 0; i < 5; i++)
			if (!SAFE_FORK())
				pause();

		/* Kill all processes in this process group */
		SAFE_KILL(0, TEST_SIG);
		pause();
	}

	/*
	 * Check to see if the process was terminated with the
	 * expected signal.
	 */
	SAFE_WAITPID(pid, &status, 0);

	nsig = WTERMSIG(status);
	if (nsig == TEST_SIG)
		tst_res(TPASS, "received expected signal %d", nsig);
	else
		tst_res(TFAIL, "expected signal %d received %d", TEST_SIG, nsig);
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
};
