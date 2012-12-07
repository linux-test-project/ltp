/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 If any of the signals listed in the array below are not is "signalset"
 after sigfillset is called on it, then fail, otherwise pass.
*/

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include "posixtest.h"

#define NUMSIGNALS (sizeof(siglist) / sizeof(siglist[0]))

int main(void)
{
	sigset_t signalset;
	int i, test_failed = 0;

	int siglist[] = { SIGABRT, SIGALRM, SIGBUS, SIGCHLD,
		SIGCONT, SIGFPE, SIGHUP, SIGILL, SIGINT,
		SIGKILL, SIGPIPE, SIGQUIT, SIGSEGV, SIGSTOP,
		SIGTERM, SIGTSTP, SIGTTIN, SIGTTOU, SIGUSR1,
		SIGUSR2,
#ifdef SIGPOLL
		SIGPOLL,
#endif
#ifdef SIGPROF
		SIGPROF,
#endif
		SIGSYS,
		SIGTRAP, SIGURG, SIGVTALRM, SIGXCPU, SIGXFSZ
	};

	if (sigfillset(&signalset) == -1) {
		perror("sigfillset failed -- test aborted");
		return PTS_FAIL;
	}

	for (i = NUMSIGNALS - 1; i >= 0; i--) {
		if (sigismember(&signalset, siglist[i]) == 0) {
#ifdef DEBUG
			printf("sigfillset did not insert signal %s\n in set",
			       siglist[i]);
#endif
			test_failed = 1;
		}
	}

	if (test_failed == 1) {
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
