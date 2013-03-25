/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

   Test the error condition of calling sigdelset() to delete a signal not
   there.
   Test steps:
   1)  Initialize an empty signal set.
   2)  Verify the SIGCHLD signal is not in the empty signal set.
   3)  Attempt to remove the SIGCHLD signal from the signal set.
   4)  Verify the SIGCHLD signal is still not in the signal set.
 */
#include <stdio.h>
#include <signal.h>
#include "posixtest.h"

int main(void)
{
	sigset_t signalset;

	if (sigemptyset(&signalset) == -1) {
		perror("sigemptyset failed -- test aborted");
		return PTS_UNRESOLVED;
	}

	if (sigismember(&signalset, SIGCHLD) == 1) {
		perror("SIGCHLD is already a member of signal set");
		return PTS_UNRESOLVED;
	}

	sigdelset(&signalset, SIGCHLD);

	if (sigismember(&signalset, SIGCHLD) == 0) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
}
