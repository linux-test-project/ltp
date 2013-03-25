/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 Make sure that none of the signals listed in this array below are
 found in "signalset" after sigemptyset() is called on it.

*/

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include "posixtest.h"

#define NUMSIGNALS (sizeof(siglist) / sizeof(siglist[0]))

int main(void)
{

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

	sigset_t signalset;
	int i, test_failed = 0;

	if (sigemptyset(&signalset) == -1) {
		perror("sigemptyset failed -- test aborted");
		return -1;
	}

	for (i = NUMSIGNALS - 1; i >= 0; i--) {
		if (sigismember(&signalset, siglist[i]) == 1) {
#ifdef DEBUG
			printf("sigemptyset did not clear set of signal %s\n",
			       siglist[i]);
#endif
			test_failed = 1;
		}
	}

	if (test_failed == 1) {
		return PTS_FAIL;
	}

	printf("sigemptyset passed\n");
	return PTS_PASS;
}
