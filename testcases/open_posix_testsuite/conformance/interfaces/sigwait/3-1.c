#include <signal.h>
#include <stdio.h>
#include "posixtest.h"

/*

 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Test that the sigwait() function.
 *  If prior to the call to sigwait() there are multiple pending instances of
 *  a single signal number (and it is implementation-defined that the signal
 *  number DOES NOT support queued signals), then there should be no remaining
 *  pending signals for that signal number.
 *  Steps are:
 *  1)  Block a signal that doesn't support queueing from delivery.
 *  2)  Raise that signal 4 times.
 *  3)  Call sigwait()
 *  4)  Verify it cleared the signal from the pending signals and there
 *      are no signals left in the pending list.
 *
 */

int main(void)
{
	sigset_t newmask, pendingset;
	int sig;

	/* Empty set of blocked signals */
	if ((sigemptyset(&newmask) == -1) || (sigemptyset(&pendingset) == -1)) {
		printf("Error in sigemptyset()\n");
		return PTS_UNRESOLVED;
	}

	/* Add SIGUSR2 to the set of blocked signals */
	if (sigaddset(&newmask, SIGUSR2) == -1) {
		perror("Error in sigaddset()\n");
		return PTS_UNRESOLVED;
	}

	/* Block SIGUSR2 */
	if (sigprocmask(SIG_SETMASK, &newmask, NULL) == -1) {
		printf("Error in sigprocmask()\n");
		return PTS_UNRESOLVED;
	}

	/* Send SIGUSR2 signal 4 times to this process.  Since it is blocked,
	 * it should be pending. */
	if (raise(SIGUSR2) != 0) {
		printf("Could not raise SIGUSR2\n");
		return PTS_UNRESOLVED;
	}
	if (raise(SIGUSR2) != 0) {
		printf("Could not raise SIGUSR2\n");
		return PTS_UNRESOLVED;
	}
	if (raise(SIGUSR2) != 0) {
		printf("Could not raise SIGUSR2\n");
		return PTS_UNRESOLVED;
	}
	if (raise(SIGUSR2) != 0) {
		printf("Could not raise SIGUSR2\n");
		return PTS_UNRESOLVED;
	}

	/* Obtain a set of pending signals */
	if (sigpending(&pendingset) == -1) {
		printf("Error calling sigpending()\n");
		return PTS_UNRESOLVED;
	}

	/* Make sure SIGUSR2 is pending */
	if (sigismember(&pendingset, SIGUSR2) == 0) {
		printf("Error: signal SIGUSR2 not pending\n");
		return PTS_UNRESOLVED;
	}

	/* Call sigwait */
	if (sigwait(&newmask, &sig) != 0) {
		printf("Error in sigwait\n");
		return PTS_UNRESOLVED;
	}

	/* Make sure SIGUSR2 is not in the pending list anymore */
	if (sigpending(&pendingset) == -1) {
		printf("Error calling sigpending()\n");
		return PTS_UNRESOLVED;
	}

	if (sigismember(&pendingset, SIGUSR2) == 1) {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;

}
