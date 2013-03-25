/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Test that the sigpending() function returns 0 on successful completion.
 *  1)  Block one signal from delivery.
 *  2)  Raise that signal.
 *  3)  Call sigpending and verify it returns 0.
 *  4)  Verify the signal raised is shown.
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

	if (sigaddset(&blockset, SIGUSR2) == -1) {
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

	if (sigpending(&pendingset) == 0) {
		if (sigismember(&pendingset, SIGUSR2) == 1) {
			printf("sigpending returned 0 when successful\n");
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("sigpending returned 0 when unsuccessful\n");
			printf("Test FAILED\n");
			return PTS_FAIL;
		}
	} else {
		printf("sigpending did not return 0\n");
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	return PTS_UNRESOLVED;
}
