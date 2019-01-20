/*

 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 Go through all the signals (with the exception of SIGKILL and SIGSTOP
 since they cannot be added to a process's signal mask) and add each one
 to the signal mask. Every time a signal gets added to the signal mask
 (using the sigprocmask() function),  make sure that all signals added
 before it in preceding iterations before it, exist in the old signal set
 returned by the sigprocmask functions.

*/

#include <signal.h>
#include <stdio.h>
#include "posixtest.h"

#define NUMSIGNALS (sizeof(siglist) / sizeof(siglist[0]))

int main(void)
{
	sigset_t oactl, tempset;
	int i, j, test_failed = 0;

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
		sigemptyset(&oactl);
		sigemptyset(&tempset);
		sigaddset(&tempset, siglist[i]);
		sigprocmask(SIG_BLOCK, &tempset, &oactl);
		if (i > 0) {
			for (j = 0; j < i; j++) {
				if (sigismember(&oactl, siglist[j]) != 1) {
					test_failed = 1;
				}
			}
		}
	}

	if (test_failed != 0) {
		printf("Old set is missing a signal\n");
		return PTS_FAIL;
	}

	printf
	    ("Test PASSED: oactl did contain all signals that were added to the signal mask.\n");
	return PTS_PASS;
}
