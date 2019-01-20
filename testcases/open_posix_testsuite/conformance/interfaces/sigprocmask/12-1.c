/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 Steps:
 1. Set the signal mask to only having SIGABRT.
 2. Call sigprocmask again, this time with a randomly generated value of
 how that is checked to make sure it does not equal any of the three defined
 values of how which are SIG_SETMASK, SIG_BLOCK, or SIG_UNBLOCK. This should
 cause sigprocmask() to return -1. For the second parameter in the sigprocmask()
 function, use a set which contains SIGABRT and SIGALRM.
 3. Now verify using the is_changed() function that the only signal that is still
 in the signal mask is SIGABRT. Neither SIGALRM nor any other signal should be
 in the signal mask of the process.

*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

#define NUMSIGNALS (sizeof(siglist) / sizeof(siglist[0]))

int is_changed(sigset_t set)
{

	int i;
	int siglist[] = { SIGALRM, SIGBUS, SIGCHLD,
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
		if (sigismember(&set, siglist[i]) != 0)
			return 1;
	}
	return 0;
}

int get_rand(void)
{

	int r;
	r = rand();
	while ((r == SIG_BLOCK) || (r == SIG_SETMASK) || (r == SIG_UNBLOCK)) {
		r = get_rand();
	}
	return r;
}

int main(void)
{

	int r = get_rand();
	sigset_t actl, oactl;

	sigemptyset(&actl);
	sigemptyset(&oactl);
	sigaddset(&actl, SIGABRT);

	sigprocmask(SIG_SETMASK, &actl, NULL);

	sigaddset(&actl, SIGALRM);
	if (sigprocmask(r, &actl, NULL) != -1) {
		perror
		    ("sigprocmask() did not fail even though invalid how parameter was passed to it.\n");
		return PTS_UNRESOLVED;
	}

	sigprocmask(SIG_SETMASK, NULL, &oactl);

	if (sigismember(&oactl, SIGABRT) != 1) {
		printf("FAIL: signal mask was changed. \n");
		return PTS_FAIL;
	}

	if (is_changed(oactl)) {
		printf("FAIL: signal mask was changed. \n");
		return PTS_FAIL;
	}

	printf("Test PASSED: signal mask was not changed.\n");
	return PTS_PASS;
}
