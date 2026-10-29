// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 */

/*\
 * Restore signals to default behavior.
 */

#include "signal.h"
#include "tst_test.h"

static int siglist[] = { SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGTRAP, SIGABRT,
	SIGBUS, SIGFPE, SIGUSR1, SIGSEGV, SIGUSR2, SIGPIPE, SIGALRM,
	SIGTERM, SIGCHLD, SIGCONT, SIGTSTP, SIGTTIN,
	SIGTTOU, SIGURG, SIGXCPU, SIGXFSZ, SIGVTALRM, SIGPROF,
	SIGWINCH, SIGIO, SIGPWR, SIGSYS
};

static void sighandler(int sig LTP_ATTRIBUTE_UNUSED)
{
}

static void do_test(unsigned int n)
{
	sighandler_t rval, first;

	SAFE_SIGNAL(siglist[n], SIG_DFL);
	first = SAFE_SIGNAL(siglist[n], sighandler);

	SAFE_SIGNAL(siglist[n], SIG_DFL);
	rval = SAFE_SIGNAL(siglist[n], sighandler);

	if (rval == first) {
		tst_res(TPASS, "signal(%d) restore succeeded "
				"received %p.", siglist[n], rval);
	} else {
		tst_res(TFAIL, "return values for signal(%d) don't match. "
				"Got %p, expected %p.",
				siglist[n], rval, first);
	}
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(siglist),
	.test = do_test,
};
