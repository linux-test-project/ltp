/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Test that the sigpending() function stores the set of signals that
 *  are blocked from delivery when there are signals blocked both
 *  in the main function and in the signal handler.
 *  1)  Block two signals from delivery to main process.
 *  2)  Block three signals from delivery to a signal handler.
 *  3)  Raise the signal to get into that signal handler.
 *  From the signal handler:
 *    4)  Raise one blocked signal in the signal handler.
 *    5)  Raise one blocked signal in the main process.
 *    5)  Verify that the two signals raised are shown via sigpending.
 *    6)  Verify the three signals not raised are not shown.
 */

/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Signal handler uses exit() to leave so that the signals are not executed.
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	sigset_t pendingset;

	if (sigemptyset(&pendingset) == -1) {
		printf("Could not call sigemptyset()\n");
		exit(-1);
	}
	if (raise(SIGUSR2) != 0) {
		printf("Could not raise SIGUSR2\n");
		exit(-1);
	}
	if (raise(SIGCONT) != 0) {
		printf("Could not raise SIGCONT\n");
		exit(-1);
	}

	if (sigpending(&pendingset) == -1) {
		printf("Error calling sigpending()\n");
		exit(-1);
	}

	if (sigismember(&pendingset, SIGUSR2) == 1 &&
	    sigismember(&pendingset, SIGCONT) == 1) {
		printf("All pending signals found\n");
		if ((sigismember(&pendingset, SIGHUP) == 0) &&
		    (sigismember(&pendingset, SIGABRT) == 0) &&
		    (sigismember(&pendingset, SIGUSR1) == 0)) {
			printf("Unsent signals not found\n");
			printf("Test PASSED\n");
			exit(0);
		} else {
			printf("Error with unsent signals\n");
			printf("Test FAILED\n");
			exit(-1);
		}
	} else {
		printf("Error with send signals\n");
		printf("Test FAILED\n");
		exit(-1);
	}
}

int main(void)
{
	sigset_t blockset;
	sigset_t prevset;
	struct sigaction act;

	act.sa_handler = handler;
	act.sa_flags = 0;

	if ((sigemptyset(&blockset) == -1) ||
	    (sigemptyset(&prevset) == -1) ||
	    (sigemptyset(&act.sa_mask) == -1)) {
		printf("Could not call sigemptyset()\n");
		return PTS_UNRESOLVED;
	}

	if ((sigaddset(&blockset, SIGUSR2) == -1) ||
	    (sigaddset(&blockset, SIGHUP) == -1)) {
		perror("Error calling sigaddset()\n");
		return PTS_UNRESOLVED;
	}

	if (sigprocmask(SIG_SETMASK, &blockset, &prevset) == -1) {
		printf("Could not call sigprocmask()\n");
		return PTS_UNRESOLVED;
	}

	if ((sigaddset(&act.sa_mask, SIGCONT) == -1) ||
	    (sigaddset(&act.sa_mask, SIGABRT) == -1) ||
	    (sigaddset(&act.sa_mask, SIGUSR1) == -1)) {
		perror("Error calling sigaddset()\n");
		return PTS_UNRESOLVED;
	}

	if (sigaction(SIGTTOU, &act, 0) == -1) {
		perror("Could not call sigaction()");
		return PTS_UNRESOLVED;
	}

	if (raise(SIGTTOU) == -1) {
		perror("Could not raise SIGTTOU");
		return PTS_UNRESOLVED;
	}
	printf("This code should not be reachable\n");
	return PTS_UNRESOLVED;
}
