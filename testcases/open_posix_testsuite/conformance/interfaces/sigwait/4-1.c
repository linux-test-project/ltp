#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include "posixtest.h"

/*

 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Test that the sigwait() function. If no signal in 'set' is pending at the
 *  time of the call, the thread shall be suspended until one or more becomes
 *  pending.
 *  1)  Block a signal from delivery.
 *  2)  Call sigwait()
 *  3)  Raise the signal.
 *  4)  Verify this process will return when the signal is sent.
 */

int main(void)
{
	sigset_t newmask, pendingset;
	int sig;

	struct timeval tv_ref, tv_cur;

	/* Empty set of blocked signals */

	if (sigemptyset(&newmask) == -1 || sigemptyset(&pendingset) == -1) {
		printf("Error in sigemptyset()\n");
		return PTS_UNRESOLVED;
	}

	/* Add SIGALRM to the set of blocked signals */
	if (sigaddset(&newmask, SIGALRM) == -1) {
		perror("Error in sigaddset()\n");
		return PTS_UNRESOLVED;
	}

	/* Block SIGALRM */
	if (sigprocmask(SIG_SETMASK, &newmask, NULL) == -1) {
		printf("Error in sigprocmask()\n");
		return PTS_UNRESOLVED;
	}

	/* Read clock */
	if (gettimeofday(&tv_ref, NULL) != 0) {
		printf("Failed to get time of day");
		return PTS_UNRESOLVED;
	}

	/* SIGALRM will be sent in 5 seconds */
	alarm(3);

	/* Call sigwait.  It should wait for 5 seconds and then move
	 * along the rest of the process when it received the SIGALRM */
	if (sigwait(&newmask, &sig) != 0) {
		printf("Error in sigwait()\n");
		return PTS_UNRESOLVED;
	}

	/* Re-read clock */
	if (gettimeofday(&tv_cur, NULL) != 0) {
		printf("Failed to get time of day");
		return PTS_UNRESOLVED;
	}

	/* Check the operation was blocking until the signal was generated */
	if (tv_cur.tv_sec - tv_ref.tv_sec < 2) {
		printf("The operation lasted less than 3 seconds!\n");
		return PTS_FAIL;
	}

	/* If we get here, then the process was suspended until
	 * SIGALRM was raised.  */
	printf("Test PASSED\n");

	return PTS_PASS;

}
