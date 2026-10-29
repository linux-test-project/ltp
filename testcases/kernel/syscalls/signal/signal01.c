// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 * AUTHOR	: Dave Baumgartner
 * CO-PILOT	: Barrie Kletscher
 */

/*\
 * Test SIGKILL for these items:
 *	1. SIGKILL can not be set to be ignored, errno:EINVAL (POSIX).
 *	2. SIGKILL can not be reset to default, errno:EINVAL (POSIX).
 *	3. SIGKILL can not be set to be caught, errno:EINVAL (POSIX).
 *	4. SIGKILL can not be ignored.
 *	5. SIGKILL is reset to default failed but processed by default.
 *	6. SIGKILL can not be caught.
 */

#include <stdlib.h>
#include "tst_test.h"

static void catchsig(int sig)
{
	(void)sig;
}

static struct tcase {
	void (*sighandler)(int i);
	int kill;
} tcases[] = {
	{SIG_IGN, 0},
	{SIG_DFL, 0},
	{catchsig, 0},
	{SIG_IGN, 1},
	{SIG_DFL, 1},
	{catchsig, 1},
};

static void do_test(unsigned int n)
{
	pid_t pid;
	int res;

	struct tcase *tc = &tcases[n];

	pid = SAFE_FORK();

	if (!pid) {
		if (tc->kill) {
			signal(SIGKILL, tc->sighandler);
			pause();
		}

		TST_EXP_FAIL2((long)signal(SIGKILL, tc->sighandler), EINVAL);
		return;
	}

	if (!tc->kill) {
		SAFE_WAITPID(pid, &res, 0);
		return;
	}

	TST_PROCESS_STATE_WAIT(pid, 'S', 0);
	SAFE_KILL(pid, SIGKILL);
	SAFE_WAITPID(pid, &res, 0);

	if (WIFSIGNALED(res))
		TST_EXP_EQ_SSZ(WTERMSIG(res), SIGKILL);
	else
		tst_res(TFAIL, "Child not killed by signal");
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.forks_child = 1,
	.test = do_test,
};
