// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Copyright (c) Manas Kumar Nayak maknayak@in.ibm.com>
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test if waitid() filters children killed with SIGSTOP.
 */

#include <sys/wait.h>
#include "tst_test.h"

static siginfo_t *infop;

static void run(void)
{
	pid_t pid_child;

	pid_child = SAFE_FORK();
	if (!pid_child) {
		SAFE_KILL(getpid(), SIGSTOP);
		TST_CHECKPOINT_WAIT(0);
		return;
	}

	memset(infop, 0, sizeof(*infop));
	TST_EXP_PASS(waitid(P_PID, pid_child, infop, WSTOPPED | WNOWAIT));

	TST_EXP_EQ_LI(infop->si_pid, pid_child);
	TST_EXP_EQ_LI(infop->si_status, SIGSTOP);
	TST_EXP_EQ_LI(infop->si_signo, SIGCHLD);
	TST_EXP_EQ_LI(infop->si_code, CLD_STOPPED);

	SAFE_KILL(pid_child, SIGCONT);

	TST_CHECKPOINT_WAKE(0);
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.bufs = (struct tst_buffers[]) {
		{&infop, .size = sizeof(*infop)},
		{}
	}
};
