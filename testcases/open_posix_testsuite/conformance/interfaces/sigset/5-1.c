/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program tests the assertion that the process's signal mask will be
 restored to the state that it was in prior to the delivery of the signal

 Steps:
 1. Empty the signal mask
 2. Deliver the signal
 3. When we return from the signal handler, verify that the signal mask
    is still empty, otherwise fail.

*/


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

#define NUMSIGNALS (sizeof(siglist) / sizeof(siglist[0]))

int is_empty(sigset_t * set)
{

	int i;
	int siglist[] = { SIGABRT, SIGALRM, SIGBUS, SIGCHLD,
		SIGCONT, SIGFPE, SIGHUP, SIGILL, SIGINT,
		SIGPIPE, SIGQUIT, SIGSEGV,
		SIGTERM, SIGTSTP, SIGTTIN, SIGTTOU,
		SIGUSR1, SIGUSR2,
#ifdef SIGPOLL
		SIGPOLL,
#endif
#ifdef SIGPROF
		SIGPROF,
#endif
		SIGSYS,
		SIGTRAP, SIGURG, SIGVTALRM, SIGXCPU, SIGXFSZ
	};

	for (i = 0; i < (int)NUMSIGNALS; i++) {
		if (sigismember(set, siglist[i]) != 0)
			return 0;
	}
	return 1;
}

void myhandler(int signo LTP_ATTRIBUTE_UNUSED)
{
}

int main(void)
{
	sigset_t mask;
	sigemptyset(&mask);

	if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1) {
		perror("sigprocmask(SIG_SETMASK, &mask, NULL) failed");
		return PTS_UNRESOLVED;
	}

	if (sigset(SIGCHLD, myhandler) == SIG_ERR) {
		perror("Unexpected error while using sigset()");
		return PTS_UNRESOLVED;
	}

	raise(SIGCHLD);
	sigprocmask(SIG_SETMASK, NULL, &mask);

	if (is_empty(&mask) != 1) {
		printf("Test FAILED: signal mask should be empty\n");
		return PTS_FAIL;
	}
	return PTS_PASS;
}
