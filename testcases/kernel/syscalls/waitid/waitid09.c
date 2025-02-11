// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Copyright (c) Manas Kumar Nayak maknayak@in.ibm.com>
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test that waitid() fails with ECHILD with process that is not child of the
 * current process. We fork() one child just to be sure that there are unwaited
 * for children available while the test runs.
 */

#include <stdlib.h>
#include <sys/wait.h>
#include "tst_test.h"

static siginfo_t *infop;

static void run(void)
{
	if (!SAFE_FORK())
		exit(0);

	TST_EXP_FAIL(waitid(P_PID, 1, infop, WEXITED), ECHILD);
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
	.bufs = (struct tst_buffers[]) {
		{&infop, .size = sizeof(*infop)},
		{}
	}
};
