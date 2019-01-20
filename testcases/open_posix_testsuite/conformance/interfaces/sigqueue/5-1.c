/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program tests the assertion that if SA_SIGINFO is not set, then the
 signal shall be received at least once

 Steps:
 - Register for myhandler to be called when SIGTOTEST is called, and make
   sure SA_SIGINFO is NOT SET.
 - Block signal SIGTOTEST from the process.
 - Using sigqueue(), send NUMCALLS instances of SIGTOTEST to the current
   process.
 - Verify that the handler is executed at least once AFTER SIGTOTEST
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

void myhandler(int signo LTP_ATTRIBUTE_UNUSED)
{
	counter++;
}

int main(void)
{
	int pid, i;
	union sigval value;
	struct sigaction act;

	act.sa_handler = myhandler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGTOTEST, &act, 0);

	value.sival_int = 0;	/* 0 is just an arbitrary value */
	pid = getpid();

	sighold(SIGTOTEST);

	for (i = 0; i < NUMCALLS; i++) {
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

	if (counter < 1) {
		printf("Test FAILED: %d was not received even once\n",
		       SIGTOTEST);
		return PTS_FAIL;
	}
	printf("Test PASSED: %d was received %d times.\n", SIGTOTEST, counter);
	return PTS_PASS;
}
