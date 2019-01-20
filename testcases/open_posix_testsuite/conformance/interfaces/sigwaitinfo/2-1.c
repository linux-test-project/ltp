/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program tests the assertion that the lowest pending signal will be
 selected by sigwaitinfo() if there are any multiple pending signals in the
 range SIGRTMIN to SIGRTMAX.

 Steps:
 - Register for myhandler to be called when any signal between SIGRTMIN
   and SIGRTMAX is generated, and make sure SA_SIGINFO is set.
 - Also, make sure that all of these signals are added to the set that
   will be passed to sigwaitinfo().
 - Block all of these signals from the process.
 - Raise all of these signals using sigqueue.
 - call sigwaitinfo() and verify that it returns SIGRTMIN
 */

#define _XOPEN_REALTIME 1

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "posixtest.h"

void myhandler(int signo LTP_ATTRIBUTE_UNUSED,
	siginfo_t *info LTP_ATTRIBUTE_UNUSED,
	void *context LTP_ATTRIBUTE_UNUSED)
{
	printf("Inside dummy handler\n");
}

int main(void)
{
	int pid, rtsig;
	union sigval value;
	struct sigaction act;
	sigset_t selectset;

	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = myhandler;
	sigemptyset(&act.sa_mask);
	sigemptyset(&selectset);

	for (rtsig = SIGRTMAX; rtsig >= SIGRTMIN; rtsig--) {
		sigaddset(&act.sa_mask, rtsig);
		sighold(rtsig);
		sigaddset(&selectset, rtsig);
	}

	pid = getpid();
	value.sival_int = 5;	/* 5 is just an arbitrary value */

	for (rtsig = SIGRTMAX; rtsig >= SIGRTMIN; rtsig--) {
		sigaction(rtsig, &act, 0);
		if (sigqueue(pid, rtsig, value) != 0) {
			printf
			    ("Test UNRESOLVED: call to sigqueue did not return success\n");
			return PTS_UNRESOLVED;
		}
	}

	if (sigwaitinfo(&selectset, NULL) != SIGRTMIN) {
		printf
		    ("Test FAILED: sigwaitinfo() did not return the lowest of the multiple pending signals between SIGRTMIN and SIGRTMAX\n");
		return PTS_FAIL;
	}

	return PTS_PASS;
}
