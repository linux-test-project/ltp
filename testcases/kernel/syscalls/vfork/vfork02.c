// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (C) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 *  Fork a process using vfork() and verify that, the pending signals in
 *  the parent are not pending in the child process.
 */

#include "tst_test.h"

static sigset_t mask;

static void run(void)
{
	if (!vfork()) {
		sigset_t signal;

		tst_res(TINFO, "child: verify if SIGUSR1 signal is not on hold");

		if (sigpending(&signal) == -1)
			tst_brk(TBROK | TERRNO, "sigpending() error");

		TST_EXP_EQ_LI(sigismember(&signal, SIGUSR1), 0);

		_exit(0);
	}
}

static void sig_handler(LTP_ATTRIBUTE_UNUSED int signo)
{
}

static void setup(void)
{
	struct sigaction action;
	sigset_t signal;

	tst_res(TINFO, "parent: hold SIGUSR1 signal");

	memset(&action, 0, sizeof(action));
	action.sa_handler = sig_handler;
	SAFE_SIGACTION(SIGUSR1, &action, NULL);

	SAFE_SIGEMPTYSET(&mask);
	SAFE_SIGADDSET(&mask, SIGUSR1);
	SAFE_SIGPROCMASK(SIG_BLOCK, &mask, NULL);

	SAFE_KILL(getpid(), SIGUSR1);

	if (sigpending(&signal) == -1)
		tst_brk(TBROK | TERRNO, "sigpending() error");

	TEST(sigismember(&signal, SIGUSR1));
	if (TST_RET != 1) {
		if (TST_RET == -1)
			tst_brk(TBROK | TERRNO, "sigismember() error");

		tst_brk(TBROK, "SIGUSR1 is not on hold");
	}
}

static void cleanup(void)
{
	SAFE_SIGEMPTYSET(&mask);
	SAFE_SIGADDSET(&mask, SIGUSR1);
	SAFE_SIGPROCMASK(SIG_UNBLOCK, &mask, NULL);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
};
