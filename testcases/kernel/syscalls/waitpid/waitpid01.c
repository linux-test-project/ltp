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
 * Fork a child that sends given signal to itself using raise() or kill(),
 * the parent checks that the signal was returned.
 */
#include <stdlib.h>
#include <sys/wait.h>
#include "tst_test.h"

static int test_coredump;
static struct testcase {
	int sig;
	int coredump;
} testcase_list[] = {
	{SIGABRT, 1},
	{SIGALRM, 0},
	{SIGBUS, 1},
	{SIGFPE, 1},
	{SIGHUP, 0},
	{SIGILL, 1},
	{SIGINT, 0},
	{SIGKILL, 0},
	{SIGPIPE, 0},
	{SIGPOLL, 0},
	{SIGPROF, 0},
	{SIGQUIT, 1},
	{SIGSEGV, 1},
	{SIGSYS, 1},
	{SIGTERM, 0},
	{SIGTRAP, 1},
	{SIGUSR1, 0},
	{SIGUSR2, 0},
	{SIGVTALRM, 0},
	{SIGXCPU, 1},
	{SIGXFSZ, 1}
};

static void child_raise(int sig)
{
	raise(sig);
	exit(0);
}

static void child_kill(int sig)
{
	kill(getpid(), sig);
	exit(0);
}

static struct testvariant {
	void (*func)(int sig);
	const char *desc;
} variant_list[] = {
	{child_raise, "raise(sig)"},
	{child_kill, "kill(getpid(), sig)"}
};

static void setup(void)
{
	struct rlimit lim = { 0 };

	/* Disable core dumps */
	SAFE_GETRLIMIT(RLIMIT_CORE, &lim);

	if (lim.rlim_max) {
		lim.rlim_cur = getpagesize();

		if (lim.rlim_max > 0 && lim.rlim_max < lim.rlim_cur)
			lim.rlim_cur = lim.rlim_max;

		SAFE_SETRLIMIT(RLIMIT_CORE, &lim);
		test_coredump = 1;
	} else {
		tst_res(TCONF, "Skipping coredump tests due to low rlimit");
	}

	tst_res(TINFO, "Testing child: %s", variant_list[tst_variant].desc);
}

static void run(unsigned int n)
{
	pid_t pid;
	int status;
	const struct testcase *tc = testcase_list + n;

	if (tc->sig != SIGKILL)
		SAFE_SIGNAL(tc->sig, SIG_DFL);

	pid = SAFE_FORK();
	if (!pid)
		variant_list[tst_variant].func(tc->sig);

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

	if (WTERMSIG(status) != tc->sig) {
		tst_res(TFAIL, "WTERMSIG() != %s but %s", tst_strsig(tc->sig),
			tst_strsig(WTERMSIG(status)));
		return;
	}

	tst_res(TPASS, "WTERMSIG() == %s", tst_strsig(tc->sig));

	if (!test_coredump)
		return;

	if (!tc->coredump) {
		if (WCOREDUMP(status))
			tst_res(TFAIL, "Child unexpectedly dumped core");

		return;
	}

	if (!WCOREDUMP(status)) {
		tst_res(TFAIL, "Child did not dump core when expected");
		return;
	}

	tst_res(TPASS, "Child dumped core as expected");
}

static struct tst_test test = {
	.forks_child = 1,
	.setup = setup,
	.test = run,
	.tcnt = ARRAY_SIZE(testcase_list),
	.test_variants = ARRAY_SIZE(variant_list),
	.needs_tmpdir = 1	/* for coredumps */
};
