/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program tests the assertion that if there aren't any more queued signals
 for the selected signal, then the pending indication of that signal is
 reset.

 Steps:
 - Register for myhandler to be called when SIGTOTEST is called, and make
   sure SA_SIGINFO is set.
 - Block signal SIGTOTEST from the process.
 - Using sigqueue(), send NUMCALLS instances of SIGTOTEST to the current
   process.
 - Call sigwaitinfo() NUMCALLS times, and verify that the pending indication
   for signal SIGTOTEST has been reset.

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

void myhandler(int signo LTP_ATTRIBUTE_UNUSED,
	siginfo_t *info LTP_ATTRIBUTE_UNUSED,
	void *context LTP_ATTRIBUTE_UNUSED)
{
	printf("Just a dummy handler\n");
}

int main(void)
{
	int pid, i;
	union sigval value;
	struct sigaction act;
	sigset_t selectset, pendingset;
	siginfo_t info;

	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = myhandler;
	sigemptyset(&act.sa_mask);
	sigaction(SIGTOTEST, &act, 0);

	pid = getpid();

	sighold(SIGTOTEST);

	for (i = NUMCALLS; i > 0; i--) {
		value.sival_int = i;
		if (sigqueue(pid, SIGTOTEST, value) != 0) {
			printf
			    ("Test FAILED: call to sigqueue did not return success\n");
			return PTS_FAIL;
		}
	}

	sigemptyset(&selectset);
	sigaddset(&selectset, SIGTOTEST);

	for (i = NUMCALLS; i > 0; i--) {
		if (sigwaitinfo(&selectset, &info) != SIGTOTEST) {
			perror
			    ("sigwaitinfo() returned signal other than SIGTOTEST\n");
			return PTS_UNRESOLVED;
		}
	}

	sigemptyset(&pendingset);
	sigpending(&pendingset);
	if (sigismember(&pendingset, SIGTOTEST) != 0) {
		printf
		    ("Test FAILED: Signal %d still pending even after call to sigwaitinfo()\n",
		     SIGTOTEST);
		return PTS_FAIL;
	}

	return PTS_PASS;
}
