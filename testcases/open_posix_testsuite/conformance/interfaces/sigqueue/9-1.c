/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 Steps:
 - Register for myhandler to be called when SIGTOTEST is called, and make
   sure SA_SIGINFO is set.
 - Block signal SIGTOTEST from the process.
 - Using sysconf(), check to see if there is a limit on number of queued
   signals that are pending. If there isn't a limit (i.e. sysconf returned
   -1), then this test is not applicable to the system's implementation,
   and thus we should pass it.
 - Using sigqueue(), send to the current process a number of instances (of SIGTOTEST)
   equal to the limit that sysconf() returned.
 - Send one more instance of SIGTOTEST and verify that sigqueue returns -1 and sets errno to
   [EAGAIN]
 */

#define _XOPEN_SOURCE 600
#define _XOPEN_REALTIME 1
#define SIGTOTEST SIGRTMIN
#define NUMCALLS 5

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "posixtest.h"

void myhandler(int signo, siginfo_t *info, void *context) {
	printf ("Inside Handler\n");
}

int main()
{

	int pid, i;
	long syslimit;
	union sigval value;
	struct sigaction act;

	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = myhandler;
	sigemptyset(&act.sa_mask);
	sigaction(SIGTOTEST, &act, 0);

	value.sival_int = 0;	/* 0 is just an arbitrary value */
	pid = getpid();

	sighold(SIGTOTEST);

	syslimit = sysconf(_SC_SIGQUEUE_MAX);

	printf("sigqueuemax %ld\n", syslimit);

	if (syslimit == -1) {
		printf("Test PASSED: Actually, test is not applicable to this implementation. This system has no defined limit."
			" Note: it is optional whether an implementation has this limit, so this is"
			" not a bug.\n");
		return PTS_PASS;
	}

	for (i=0; i<syslimit; i++) {
		if (sigqueue(pid, SIGTOTEST, value) != 0) {
			printf("Test UNRESOLVED: call to sigqueue did not return success\n");
			return PTS_UNRESOLVED;
		}
	}

	if (sigqueue(pid, SIGTOTEST, value) != -1) {
		printf("Test FAILED: sigqueue did not return -1 even though the process has already queued {SIGQUEUE_MAX} signals that are still pending.\n");
		return PTS_FAIL;
	}

	if (errno != EAGAIN) {
		printf("Test FAILED: errno was not set to [EAGAIN] even though the process has already queued {SIGQUEUE_MAX} signals that are still pending.\n");
	}

	return PTS_PASS;
}