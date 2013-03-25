/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Test that the sigpending() function stores the set of signals that
 *  are blocked from delivery.  Steps are:
 *  1)  Block three signals from delivery.
 *  2)  Raise two of those signals.
 *  3)  Verify that the two signals raised are shown via sigpending.
 *  4)  Verify the one signal not raised is not shown.
 */

#include <signal.h>
#include <stdio.h>
#include "posixtest.h"

int main(void)
{
	sigset_t blockset;
	sigset_t prevset;
	sigset_t pendingset;

	if ((sigemptyset(&blockset) == -1) ||
	    (sigemptyset(&prevset) == -1) || (sigemptyset(&pendingset) == -1)) {
		printf("Could not call sigemptyset()\n");
		return PTS_UNRESOLVED;
	}

	if ((sigaddset(&blockset, SIGUSR2) == -1) ||
	    (sigaddset(&blockset, SIGHUP) == -1) ||
	    (sigaddset(&blockset, SIGQUIT) == -1)) {
		perror("Error calling sigaddset()\n");
		return PTS_UNRESOLVED;
	}

	if (sigprocmask(SIG_SETMASK, &blockset, &prevset) == -1) {
		printf("Could not call sigprocmask()\n");
		return PTS_UNRESOLVED;
	}

	if (raise(SIGUSR2) != 0) {
		printf("Could not raise SIGUSR2\n");
		return PTS_UNRESOLVED;
	}
	if (raise(SIGQUIT) != 0) {
		printf("Could not raise SIGQUIT\n");
		return PTS_UNRESOLVED;
	}

	if (sigpending(&pendingset) == -1) {
		printf("Error calling sigpending()\n");
		return PTS_UNRESOLVED;
	}

	if (sigismember(&pendingset, SIGUSR2) == 1) {
		if (sigismember(&pendingset, SIGQUIT) == 1) {
			printf("All pending signals found\n");
			if (sigismember(&pendingset, SIGHUP) == 0) {
				printf("Unsent signals not found.\n");
				printf("Test PASSED\n");
				return PTS_PASS;
			} else {
				printf("Error with unsent signals\n");
				printf("Test FAILED\n");
				return PTS_FAIL;
			}
		}
	}
	printf("Not all pending signals found\n");
	return PTS_FAIL;
}
