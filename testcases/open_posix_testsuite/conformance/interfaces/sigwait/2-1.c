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
 *  number supports queued signals), then there should be remaining
 *  pending signals for that signal number.
 *  Steps are:
 *  1)  Block a signal that supports queueing (the realtime signal SIGRTMIN).
 *  2)  Raise that signal 2 times.
 *  3)  Call sigwait()
 *  4)  Verify that is cleared only one instance of the signal in the pending
 *      set.
 *  5) Call sigwait() again to clear the second instance of the signal in the
 *     pending list.
 *  6) Verify that there are no more instances for SIGRTMIN in the pending list. *
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

	/* Add SIGRTMIN to the set of blocked signals */
	if (sigaddset(&newmask, SIGRTMIN) == -1) {
		perror("Error in sigaddset()\n");
		return PTS_UNRESOLVED;
	}

	/* Obtain a set of pending signals and make sure SIGRTMIN
	 * isn't pending. */
	if (sigpending(&pendingset) == -1) {
		printf("Error calling sigpending()\n");
		return PTS_UNRESOLVED;
	}

	if (sigismember(&pendingset, SIGRTMIN) == 1) {
		printf("Error: signal SIGRTMIN is pending\n");
		return PTS_UNRESOLVED;
	}

	/* Block SIGALRM */
	if (sigprocmask(SIG_SETMASK, &newmask, NULL) == -1) {
		printf("Error in sigprocmask()\n");
		return PTS_UNRESOLVED;
	}

	/* Send SIGALRM signal 2 times to this process.  Since it is blocked,
	 * it should be pending and queued. */
	if (raise(SIGRTMIN) != 0) {
		printf("Could not raise SIGALRM\n");
		return PTS_UNRESOLVED;
	}

	if (raise(SIGRTMIN) != 0) {
		printf("Could not raise SIGALRM\n");
		return PTS_UNRESOLVED;
	}

	/* Obtain a set of pending signals */
	if (sigpending(&pendingset) == -1) {
		printf("Error calling sigpending()\n");
		return PTS_UNRESOLVED;
	}

	/* Make sure SIGRTMIN is still pending since sigwait should have only
	 * deleted one instance of SIGRTMIN from the pending set. */
	if (sigismember(&pendingset, SIGRTMIN) == 0) {
		printf("Test FAILED\n");
		return -1;
	}

	/* Call sigwait to remove first SIGRTMIN instance from the
	 * pending list. */
	if (sigwait(&newmask, &sig) != 0) {
		printf("Error in sigwait\n");
		return PTS_FAIL;
	}

	/* Make sure SIGRTMIN is still in the pending list */
	if (sigpending(&pendingset) == -1) {
		printf("Error calling sigpending()\n");
		return PTS_UNRESOLVED;
	}

	if (sigismember(&pendingset, SIGRTMIN) == 0) {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	/* Call sigwait again to remove last SIGRTMIN instance from the
	 * pending list. */
	if (sigwait(&newmask, &sig) != 0) {
		printf("Error in sigwait\n");
		return PTS_FAIL;
	}

	/* Make sure SIGRTMIN is NOT in the pending list anymore, since
	 * the previous sigwait() should have taken it out of the
	 * pending list. */
	if (sigpending(&pendingset) == -1) {
		printf("Error calling sigpending()\n");
		return PTS_UNRESOLVED;
	}

	if (sigismember(&pendingset, SIGRTMIN) != 0) {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;

}
