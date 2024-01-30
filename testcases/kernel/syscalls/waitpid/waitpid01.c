// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 *    07/2001 John George
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * [Description]
 *
 * Check that when a child kills itself with one of the standard signals,
 * the waiting parent is correctly notified.
 *
 * Fork a child that raise()s given signal, the parent checks that the signal
 * was returned.
 */
#include <stdlib.h>
#include <sys/wait.h>
#include "tst_test.h"

static int testcase_list[] = {
	SIGABRT,
	SIGALRM,
	SIGBUS,
	SIGFPE,
	SIGHUP,
	SIGILL,
	SIGINT,
	SIGKILL,
	SIGPIPE,
	SIGPOLL,
	SIGPROF,
	SIGQUIT,
	SIGSEGV,
	SIGSYS,
	SIGTERM,
	SIGTRAP,
	SIGUSR1,
	SIGUSR1,
	SIGVTALRM,
	SIGXCPU,
	SIGXFSZ
};

static void setup(void)
{
	struct rlimit lim = { 0 };

	/* Disable core dumps */
	SAFE_SETRLIMIT(RLIMIT_CORE, &lim);
}

static void run(unsigned int n)
{
	pid_t pid;
	int status, sig = testcase_list[n];

	pid = SAFE_FORK();
	if (!pid) {
		raise(sig);
		exit(0);
	}

	TST_EXP_PID_SILENT(waitpid(pid, &status, 0));
	if (!TST_PASS)
		return;

	if (TST_RET != pid) {
		tst_res(TFAIL, "waitpid() returned wrong pid %li, expected %i",
			TST_RET, pid);
	} else {
		tst_res(TPASS, "waitpid() returned correct pid %i", pid);
	}

	if (!WIFSIGNALED(status)) {
		tst_res(TFAIL, "WIFSIGNALED() not set in status (%s)",
			tst_strstatus(status));
		return;
	}

	tst_res(TPASS, "WIFSIGNALED() set in status");

	if (WTERMSIG(status) != sig) {
		tst_res(TFAIL, "WTERMSIG() != %s but %s", tst_strsig(sig),
			tst_strsig(WTERMSIG(status)));
		return;
	}

	tst_res(TPASS, "WTERMSIG() == %s", tst_strsig(sig));
}

static struct tst_test test = {
	.forks_child = 1,
	.setup = setup,
	.test = run,
	.tcnt = ARRAY_SIZE(testcase_list)
};
