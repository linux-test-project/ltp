#include <signal.h>
#include <stdio.h>
#include "posixtest.h"

/*

 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Test that the sigwait() function. Upon successful completion, sigwait()
 *  returns 0 and stores the signal number of the received signal at the
 *  location referenced by 'sig'.
 *  1)  Block a signal from delivery.
 *  2)  Call sigwait()
 *  3)  Raise the signal (to continue the process so we don't pause with
 *      sigwait() forever.
 *  4)  Verify the return value and the 'sig' value is correct.
 */

int main(void)
{
	sigset_t newmask, pendingset;
	int sig;

	/* Empty set of blocked signals */
	if (sigemptyset(&newmask) == -1 || sigemptyset(&pendingset) == -1) {
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

	/* Send SIGUSR2 signal to this process.  Since it is blocked,
	 * it should be pending */
	if (raise(SIGUSR2) != 0) {
		printf("Could not raise SIGUSR2\n");
		return PTS_UNRESOLVED;
	}

	/* Call sigwait and test if it passed/failed */
	if (sigwait(&newmask, &sig) != 0) {
		printf("Error in sigwait()\n");
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	/* If we get here, then the process was suspended until
	 * SIGUSR2 was raised.  */
	if (sig == SIGUSR2) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

}
