/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Test that the the raise() function does not return until after the
 *  signal handler it calls returns.
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

/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Declare a global variable to keep track of where we were.
 */
#define BEFOREHANDLER 1
#define INHANDLER 2
#define LEAVINGHANDLER 3
int globalStatus = BEFOREHANDLER;

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	globalStatus = INHANDLER;
	printf("Caught signal being tested!\n");
	printf("Sleeping...\n");
	sleep(2);
	globalStatus = LEAVINGHANDLER;
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
	if (raise(SIGTOTEST) != 0) {
		printf("Could not raise signal being tested\n");
		return PTS_FAIL;
	}

	if (INHANDLER == globalStatus || BEFOREHANDLER == globalStatus) {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	if (LEAVINGHANDLER == globalStatus) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("Should not reach here\n");
	return PTS_UNRESOLVED;
}
