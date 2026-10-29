// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2001
 */

/*\
 * Set signals to be ignored.
 */

#include "tst_test.h"

static int siglist[] = { SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGTRAP, SIGABRT, SIGIOT,
	SIGBUS, SIGFPE, SIGUSR1, SIGSEGV, SIGUSR2, SIGPIPE, SIGALRM,
	SIGTERM,
#ifdef SIGSTKFLT
	SIGSTKFLT,
#endif
	SIGCHLD, SIGCONT, SIGTSTP, SIGTTIN,
	SIGTTOU, SIGURG, SIGXCPU, SIGXFSZ, SIGVTALRM, SIGPROF,
	SIGWINCH, SIGIO, SIGPWR, SIGSYS,
#ifdef SIGUNUSED
	SIGUNUSED
#endif
};

static volatile int ign_handler;

static void sighandler(int sig LTP_ATTRIBUTE_UNUSED)
{
	ign_handler = 1;
}

static void do_test(unsigned int n)
{
	pid_t pid;

	SAFE_SIGNAL(siglist[n], sighandler);
	SAFE_SIGNAL(siglist[n], SIG_IGN);

	pid = getpid();

	SAFE_KILL(pid, siglist[n]);
	TST_EXP_EQ_SSZ(ign_handler, 0);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(siglist),
	.test = do_test,
};
