// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2009-2019
 * Copyright (C) 2009, Ngie Cooper
 * Copyright (c) 2023 Wei Gao <wegao@suse.com>
 */

/*\
 * [Description]
 *
 * This test ptraces itself as per arbitrarily specified signals,
 * over 0 to SIGRTMAX range.
 */

#include <stdlib.h>
#include <sys/ptrace.h>
#include "lapi/signal.h"
#include "tst_test.h"

static int expect_stop;

static void test_signal(int signum)
{
	int status;
	pid_t child;

	child = SAFE_FORK();

	if (!child) {
		TST_EXP_PASS_SILENT(ptrace(PTRACE_TRACEME, 0, NULL, NULL));
		tst_res(TDEBUG, "[child] Sending kill(.., %s)", tst_strsig(signum));
		SAFE_KILL(getpid(), signum);
		exit(0);
	}

	SAFE_WAITPID(child, &status, 0);

	switch (signum) {
	case 0:
		if (WIFEXITED(status)
				&& WEXITSTATUS(status) == 0) {
			tst_res(TPASS,
					"kill(.., 0) exited with 0, as expected.");
		} else {
			tst_res(TFAIL,
					"kill(.., 0) exited with unexpected %s.", tst_strstatus(status));
		}
		break;
	case SIGKILL:
		if (WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL)
			tst_res(TPASS, "Child killed by SIGKILL");
		else
			tst_res(TFAIL, "Child %s", tst_strstatus(status));
		break;
		/* All other processes should be stopped. */
	default:
		if (WIFSTOPPED(status)) {
			tst_res(TDEBUG, "Stopped as expected");
		} else {
			tst_res(TFAIL, "Didn't stop as expected. Child %s", tst_strstatus(status));
			expect_stop++;
		}
		break;
	}

	if (signum != 0 && signum != SIGKILL)
		SAFE_PTRACE(PTRACE_CONT, child, NULL, NULL);
}

static void run(void)
{
	int signum = 0;

	for (signum = 0; signum <= SIGRTMAX; signum++) {
		if (signum >= __SIGRTMIN && signum < SIGRTMIN)
			continue;
		test_signal(signum);
	}
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
};
