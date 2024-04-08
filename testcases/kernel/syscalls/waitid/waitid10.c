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

	/*
	 * Triggering SIGFPE by invalid instruction is not always possible,
	 * some architectures does not trap division-by-zero at all and even
	 * when it's possible we would have to fight the compiler optimizations
	 * that have tendency to remove undefined operations.
	 */
	pidchild = SAFE_FORK();
	if (!pidchild)
		raise(SIGFPE);

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
	char c;

	SAFE_GETRLIMIT(RLIMIT_CORE, &rlim);
	SAFE_FILE_SCANF("/proc/sys/kernel/core_pattern", "%c", &c);

	if (rlim.rlim_cur)
		return;

	if (!rlim.rlim_max) {
		if (c != '|')
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
	.needs_tmpdir = 1,
};
