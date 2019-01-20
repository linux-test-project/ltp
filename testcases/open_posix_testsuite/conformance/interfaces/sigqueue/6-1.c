/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program verifies that if the value of pid causes signo to be
 generated for the sending process, and if signo is not blocked for
 the calling thread and if no other thread has signo unblocked or is
 waiting in a sigwait() function for signo, then signal SIGTOTEST
 is delivered to the calling thread before the sigqueue() function returns.

 Steps:
 - Register for myhandler to be called when SIGTOTEST is called, and make
   sure SA_SIGINFO is set.
 - Using sigqueue(), send SIGTOTEST to the current process.
 - Inside handler, verify that the global return_val variable has not been
   set yet to the return value of sigqueue. If it has, then that means that
   sigqueu has returned before the handler finished executing, and thus is
   a FAILED test.
 - Also before the program ends, verify that the handler
   has been called.
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

static int return_val = 1;
static volatile int handler_called;

void myhandler(int signo LTP_ATTRIBUTE_UNUSED,
	siginfo_t *info LTP_ATTRIBUTE_UNUSED,
	void *context LTP_ATTRIBUTE_UNUSED)
{
	handler_called = 1;
	if (return_val != 1) {
		printf
		    ("Test FAILED: sigqueue() seems to have returned before handler finished executing.\n");
		exit(1);
	}
}

int main(void)
{
	int pid;
	union sigval value;
	struct sigaction act;

	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = myhandler;
	sigemptyset(&act.sa_mask);
	sigaction(SIGTOTEST, &act, 0);

	value.sival_int = 0;	/* 0 is just an arbitrary value */
	pid = getpid();

	if ((return_val = sigqueue(pid, SIGTOTEST, value)) != 0) {
		printf
		    ("Test UNRESOLVED: call to sigqueue did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (handler_called != 1) {
		printf("Test FAILED: signal was not delivered to process\n");
		return PTS_FAIL;
	}
	return PTS_PASS;
}
