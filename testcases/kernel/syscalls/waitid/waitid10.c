// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * This test is checking if waitid() syscall recognizes a process that ended
 * with division by zero error.
 */

#include <stdlib.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include "tst_test.h"

static siginfo_t *infop;
static int core_dumps = 1;

static void run(void)
{
	pid_t pidchild;

	pidchild = SAFE_FORK();
	if (!pidchild) {
		volatile int a, zero = 0;

		a = 1 / zero;
		exit(a);
	}

	TST_EXP_PASS(waitid(P_ALL, 0, infop, WEXITED));
	TST_EXP_EQ_LI(infop->si_pid, pidchild);
	TST_EXP_EQ_LI(infop->si_status, SIGFPE);
	TST_EXP_EQ_LI(infop->si_signo, SIGCHLD);

	if (core_dumps)
		TST_EXP_EQ_LI(infop->si_code, CLD_DUMPED);
	else
		TST_EXP_EQ_LI(infop->si_code, CLD_KILLED);
}

static void setup(void)
{
	struct rlimit rlim;

	SAFE_GETRLIMIT(RLIMIT_CORE, &rlim);

	if (rlim.rlim_cur)
		return;

	if (!rlim.rlim_max) {
		core_dumps = 0;
		return;
	}

	tst_res(TINFO, "Raising RLIMIT_CORE rlim_cur=%li -> %li",
	        rlim.rlim_cur, rlim.rlim_max);

	rlim.rlim_cur = rlim.rlim_max;
	SAFE_SETRLIMIT(RLIMIT_CORE, &rlim);
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
	.setup = setup,
	.bufs =	(struct tst_buffers[]) {
		{&infop, .size = sizeof(*infop)},
		{},
	},
};
