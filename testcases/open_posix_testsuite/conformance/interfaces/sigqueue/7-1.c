/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program tests the assertion that the lowest pending signal will be
 selected for delivery if there are any multiple pending signals in the
 range SIGRTMIN to SIGRTMAX.

 Steps:
 - Register for myhandler to be called when any signal between SIGRTMIN
   and SIGRTMAX is generated, and make
   sure SA_SIGINFO is set.
 - Also, make sure that all of these signals are added to the handler's
   signal mask.
 - Initially block all of these signals from the process.
 - Raise all of these signals using sigqueue.
 - Unblock all of these queued signals simultaneously using sigprocmask.
 - Verify that the signals are delivered in order from smallest to
   biggest.
 */

#define _XOPEN_REALTIME 1

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "posixtest.h"

static int last_signal;
static volatile int test_failed;

void myhandler(int signo, siginfo_t *info LTP_ATTRIBUTE_UNUSED,
	void *context LTP_ATTRIBUTE_UNUSED)
{
	printf("%d, ", signo);
	if (last_signal >= signo) {
		test_failed = 1;
	}
}

int main(void)
{
	int pid, rtsig;
	union sigval value;
	struct sigaction act;
	sigset_t mask;

	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = myhandler;
	sigemptyset(&act.sa_mask);

	sigemptyset(&mask);

	for (rtsig = SIGRTMAX; rtsig >= SIGRTMIN; rtsig--) {
		sigaddset(&act.sa_mask, rtsig);
		sighold(rtsig);
		sigaddset(&mask, rtsig);
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

	sigprocmask(SIG_UNBLOCK, &mask, NULL);
	printf("\n");

	if (test_failed == 1) {
		printf
		    ("Test FAILED: A pending signal was delivered even though a smaller one is still pending.\n");
		return PTS_FAIL;
	}

	return PTS_PASS;
}
