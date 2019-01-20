/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Assertion 4 - that raise() is equivalent to kill(getpid(), sig); is
 *  essentially tested implicitly via assertion 1.
 *  This test is the reverse test case:  Test assertion 1, but replace
 *  raise() with kill(getpid(), sig).  It should pass if assertion 1
 *  passes.
 *  1) Set up a signal handler for the signal that says we have caught the
 *     signal.
 *  2) Call kill(getpid(), <signal under test>)
 *  3) If signal handler was called, test passed.
 *  This test is only performed on one signal.  All other signals are
 *  considered to be in the same equivalence class.
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "posixtest.h"

#define SIGTOTEST SIGABRT

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("Caught signal being tested!\n");
	printf("Test PASSED\n");
	exit(0);
}

int main(void)
{
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
	if (kill(getpid(), SIGTOTEST) != 0) {
		printf("Could not call kill\n");
		return PTS_UNRESOLVED;
	}

	printf("Should have exited from signal handler\n");
	printf("Test FAILED\n");
	return PTS_FAIL;
}
