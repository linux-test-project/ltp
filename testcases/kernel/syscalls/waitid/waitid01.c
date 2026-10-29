// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * This test is checking if waitid() syscall does wait for WEXITED and check for
 * the return value.
 */

#include <stdlib.h>
#include <sys/wait.h>
#include "tst_test.h"

static siginfo_t *infop;

static void run(void)
{
	pid_t pidchild;

	pidchild = SAFE_FORK();
	if (!pidchild)
		exit(123);

	TST_EXP_PASS(waitid(P_ALL, 0, infop, WEXITED));
	TST_EXP_EQ_LI(infop->si_pid, pidchild);
	TST_EXP_EQ_LI(infop->si_status, 123);
	TST_EXP_EQ_LI(infop->si_signo, SIGCHLD);
	TST_EXP_EQ_LI(infop->si_code, CLD_EXITED);
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
	.bufs = (struct tst_buffers[]) {
		{&infop, .size = sizeof(*infop)},
		{},
	},
};
