// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Copyright (c) Manas Kumar Nayak maknayak@in.ibm.com>
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * This test if waitid() syscall leaves the si_pid set to 0 with WNOHANG flag
 * when no child was waited for.
 */

#include <sys/wait.h>
#include "tst_test.h"

static siginfo_t *infop;

static void run(void)
{
	pid_t pid_child;

	pid_child = SAFE_FORK();
	if (!pid_child) {
		TST_CHECKPOINT_WAIT(0);
		return;
	}

	memset(infop, 0, sizeof(*infop));
	TST_EXP_PASS(waitid(P_ALL, pid_child, infop, WNOHANG | WEXITED));

	TST_EXP_EQ_LI(infop->si_pid, 0);

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
