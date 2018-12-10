/*
 * Copyright (c) 2002-3, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Test that the killpg() function shall send signal sig to the process
    group specified by prgp.
    Steps:
 *  1. Set up a signal handler for the signal that says we have caught the
 *     signal.
 *  2. Call killpg on the current process group id, raising the signal.
 *  3. If signal handler was called, test passed.

 */

#define SIGTOTEST SIGCHLD

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

void handler(int signo)
{
	(void) signo;

	printf("Caught signal being tested!\n");
	printf("Test PASSED\n");
	_exit(PTS_PASS);
}

int main(void)
{
	int pgrp;
	struct sigaction act;

	act.sa_handler = handler;
	act.sa_flags = 0;
	if (sigemptyset(&act.sa_mask) == -1) {
		perror("Error calling sigemptyset\n");
		return PTS_UNRESOLVED;
	}
	if (sigaction(SIGTOTEST, &act, 0) == -1) {
		perror("Error calling sigaction\n");
		return PTS_UNRESOLVED;
	}

	if ((pgrp = getpgrp()) == -1) {
		printf("Could not get process group number\n");
		return PTS_UNRESOLVED;
	}

	if (killpg(pgrp, SIGTOTEST) != 0) {
		printf("Could not raise signal being tested\n");
		return PTS_UNRESOLVED;
	}

	printf("Should have exited from signal handler\n");
	printf("Test FAILED\n");
	return PTS_FAIL;
}
