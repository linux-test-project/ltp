/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Test that the raise(<signal>) function shall send the signal
 *  to the executing process when the executing process is a child
 *  process.
 *  1) Set up a signal handler for a signal in the parent process.
 *     This handler returns failure if called.
 *  2) Fork a child process.
 *  2) In the child process:
 *     3) Set up a signal handler for the same signal as in the parent process.
 *        This handler returns success if called.
 *     4) Raise the signal.
 *     5) If signal handler was called, return 1 (so WEXITSTATUS can test
 *        for success).
 *  6) In parent, if 1 was received, test passed.
 *  This test is only performed on one signal.  All other signals are
 *  considered to be in the same equivalence class.
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include "posixtest.h"

#define SIGTOTEST SIGABRT

void parenthandler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("Caught signal from parent!\n");
	exit(-1);
}

void childhandler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("Caught signal from child!\n");
	exit(0);
}

int main(void)
{
	struct sigaction parentact;

	parentact.sa_handler = parenthandler;
	parentact.sa_flags = 0;
	if (sigemptyset(&parentact.sa_mask) == -1) {
		perror("Error calling sigemptyset\n");
		return PTS_UNRESOLVED;
	}
	if (sigaction(SIGTOTEST, &parentact, 0) == -1) {
		perror("Error calling sigaction\n");
		return PTS_UNRESOLVED;
	}

	if (fork() == 0) {
		/* child here */
		struct sigaction childact;

		childact.sa_handler = childhandler;
		childact.sa_flags = 0;
		if (sigemptyset(&childact.sa_mask) == -1) {
			perror("Error calling sigemptyset\n");
			return PTS_UNRESOLVED;
		}
		if (sigaction(SIGTOTEST, &childact, 0) == -1) {
			perror("Error calling sigaction\n");
			return PTS_UNRESOLVED;
		}
		if (raise(SIGTOTEST) != 0) {
			printf("Could not raise signal being tested\n");
			return PTS_FAIL;
		}

		printf("Should have exited from signal handler\n");
		return PTS_FAIL;
	} else {
		/* parent here */
		int i;

		if (wait(&i) == -1) {
			perror("Error waiting for child to exit\n");
			return PTS_UNRESOLVED;
		}
		if (!WEXITSTATUS(i)) {
			printf("Child exited normally\n");
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("Child did not exit normally.\n");
			printf("Test FAILED\n");
			return PTS_FAIL;
		}
	}

	printf("Should not make it here.\n");
	return PTS_UNRESOLVED;
}
