/*

 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 Steps:
 1. Add only SIGABRT to the signal mask.
 2. Make a call such as this: sigprocmask(SIG_UNBLOCK, NULL, &oactl). At
 this point, we have obtained the signal mask in oactl.
 3. Now call is_changed to make sure that SIGABRT is still in oactl, and
 that no other signal in the set is in oactl.

*/

#include <signal.h>
#include <stdio.h>
#include "posixtest.h"

#define NUMSIGNALS (sizeof(siglist) / sizeof(siglist[0]))

int is_changed(sigset_t set, int sig)
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

	if (sigismember(&set, sig) != 1) {
		return 1;
	}
	for (i = 0; i < (int)NUMSIGNALS; i++) {
		if ((siglist[i] != sig)) {
			if (sigismember(&set, siglist[i]) != 0) {
				return 1;
			}
		}
	}
	return 0;
}

int main(void)
{
	sigset_t actl, oactl;

	sigemptyset(&actl);
	sigemptyset(&oactl);

	sigaddset(&actl, SIGABRT);

	sigprocmask(SIG_SETMASK, &actl, NULL);
	sigprocmask(SIG_UNBLOCK, NULL, &oactl);

	if (is_changed(oactl, SIGABRT))
		return PTS_FAIL;

	printf("Test PASSED: signal mask was not changed.\n");
	return PTS_PASS;
}
