// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Copyright (c) Manas Kumar Nayak maknayak@in.ibm.com>
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Tests if waitid() filters children correctly by the PID.
 *
 * - waitid() with PID + 1 returns ECHILD
 * - waitid() with PID returns correct data
 */

#include <stdlib.h>
#include <sys/wait.h>
#include "tst_test.h"

static siginfo_t *infop;

static void run(void)
{
	pid_t pid_child;

	pid_child = SAFE_FORK();
	if (!pid_child)
		exit(0);

	TST_EXP_FAIL(waitid(P_PID, pid_child+1, infop, WEXITED), ECHILD);

	memset(infop, 0, sizeof(*infop));
	TST_EXP_PASS(waitid(P_PID, pid_child, infop, WEXITED));

	TST_EXP_EQ_LI(infop->si_pid, pid_child);
	TST_EXP_EQ_LI(infop->si_status, 0);
	TST_EXP_EQ_LI(infop->si_signo, SIGCHLD);
	TST_EXP_EQ_LI(infop->si_code, CLD_EXITED);
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
	.bufs = (struct tst_buffers[]) {
		{&infop, .size = sizeof(*infop)},
		{}
	}
};
