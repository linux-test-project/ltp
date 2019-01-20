/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Test that the sigwaitinfo() function shall select the pending signal
    from the set set, and atomically clear it from the system's set of
    pending signals.

    Steps:
    1. Register signal SIGTOTEST with the handler myhandler
    2. Block SIGTOTEST from the process
    3. Raise the signal, causing it to be pending
    4. Call sigwaitinfo() and verify that the signal is no longer pending.
 */

#define _XOPEN_REALTIME 1
#define SIGTOTEST SIGUSR1

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "posixtest.h"

void myhandler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("Inside handler\n");
}

int main(void)
{

	struct sigaction act;
	sigset_t pendingset, selectset;

	act.sa_flags = 0;
	act.sa_handler = myhandler;

	sigemptyset(&pendingset);
	sigemptyset(&selectset);
	sigaddset(&selectset, SIGTOTEST);
	sigemptyset(&act.sa_mask);

	sigaction(SIGTOTEST, &act, 0);
	sighold(SIGTOTEST);
	raise(SIGTOTEST);

	sigpending(&pendingset);

	if (sigismember(&pendingset, SIGTOTEST) != 1) {
		perror("SIGTOTEST is not pending\n");
		return PTS_UNRESOLVED;
	}

	if (sigwaitinfo(&selectset, NULL) == -1) {
		perror("Call to sigwaitinfo() failed\n");
		return PTS_UNRESOLVED;
	}

	sigemptyset(&pendingset);
	sigpending(&pendingset);

	if (sigismember(&pendingset, SIGTOTEST) != 0) {
		printf
		    ("Test FAILED: Signal %d still pending even after call to sigwaitinfo()\n",
		     SIGTOTEST);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
