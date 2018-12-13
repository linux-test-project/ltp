/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program tests the assertion that if SA_SIGINFO is set, then the
 signal shall be queued to the receiving process.

 Steps:
 - Register for myhandler to be called when SIGTOTEST is called, and make
   sure SA_SIGINFO is set.
 - Block signal SIGTOTEST from the process.
 - Using sigqueue(), send NUMCALLS instances of SIGTOTEST to the current
   process.
 - Verify that the handler is executed NUMCALLS times AFTER SIGTOTEST
   is unblocked.
 */

#define _XOPEN_REALTIME 1
#define SIGTOTEST SIGRTMIN
#define NUMCALLS 5

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "posixtest.h"

int counter = 0;

void myhandler(int signo, siginfo_t * info, void *context)
{
	counter++;
}

int main(void)
{
	int pid, i;
	union sigval value;
	struct sigaction act;

	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = myhandler;
	sigemptyset(&act.sa_mask);
	sigaction(SIGTOTEST, &act, 0);

	pid = getpid();

	sighold(SIGTOTEST);

	for (i = 0; i < NUMCALLS; i++) {
		value.sival_int = i;	/* i is just an arbitrary value */
		if (sigqueue(pid, SIGTOTEST, value) != 0) {
			printf
			    ("Test UNRESOLVED: call to sigqueue did not return success\n");
			return PTS_UNRESOLVED;
		}
	}

	if (0 != counter) {
		printf
		    ("Test UNRESOLVED: handler called even though %d has not been removed from the signal mask\n",
		     SIGTOTEST);
		return PTS_UNRESOLVED;
	}

	sigrelse(SIGTOTEST);

	if (NUMCALLS != counter) {
		printf
		    ("Test FAILED: %d was queued %d time(s) even though sigqueue was called %d time(s) for %d\n",
		     SIGTOTEST, counter, NUMCALLS, SIGTOTEST);
		return PTS_FAIL;
	}
	return PTS_PASS;
}
