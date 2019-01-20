/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  General API test of the raise() function.
 *  Test calling raise with the following values:
 *  - MINIMUM valid (in this case SIGABRT signal)
 *  - MAXIMUM valid (in this case SIGXFSZ signal)
 *  Item with POSIX default action as T - SIGALRM
 *  Item with POSIX default action as A - (done - SIGABRT)
 *  Item with POSIX default action as I - SIGCHLD
 *  Item with POSIX default action as S - SIGTSTP
 *  Item with POSIX default action as C - SIGCONT
 *
 *  Error conditions:
 *  Boundary values for int
 *  MIN INT = INT32_MIN
 *  MAX INT = INT32_MAX
 *  MIN INT - 1 = 2147483647 (this is what gcc will set to)
 *  MAX INT + 1 = -2147483647 (this is what gcc will set to)
 *  unassigned value = -1073743192 (ex. of what gcc will set to)
 *  unassigned value = 1073743192 (ex. of what gcc will set to)
 *
 *  Steps:
 *  1) Set up signal handlers for all valid signals
 *  2) Call all valid signals and verify they are received.
 *  3) Call all invalid signals and verify raise() returns 0 and sets
 *     errno to EINVAL.
 *
 *  Note:  This test case maps to multiple assertions: 1, 2 (implicitly),
 *  5, 6, 7, so it was given a large number as a file name.
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include "posixtest.h"

#define NUMVALIDTESTS 6
#define NUMINVALIDTESTS 6
static int valid_tests[NUMVALIDTESTS] = {
	SIGABRT, SIGXFSZ, SIGALRM, SIGCHLD, SIGTSTP, SIGCONT
};

static int invalid_tests[NUMINVALIDTESTS] = {
	INT32_MIN, INT32_MAX, 2147483647, -2147483647, -1073743192, 1073743192
};

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("Caught signal being tested!\n");
}

int main(void)
{
	int i;
	int failure = 0;
	struct sigaction act;

	act.sa_handler = handler;
	act.sa_flags = 0;
	if (sigemptyset(&act.sa_mask) == -1) {
		perror("Error calling sigemptyset\n");
		return PTS_UNRESOLVED;
	}

	for (i = 0; i < NUMVALIDTESTS; i++) {
		if (sigaction(valid_tests[i], &act, 0) == -1) {
			perror("Error calling sigaction\n");
			return PTS_UNRESOLVED;
		}
	}

	for (i = 0; i < NUMVALIDTESTS; i++) {
		if (raise(valid_tests[i]) != 0) {
			printf("Could not raise signal being tested\n");
			failure = 1;
		}
	}

	for (i = 0; i < NUMINVALIDTESTS; i++) {
		if (raise(invalid_tests[i]) == 0) {
			printf("Received success raising an invalid signal\n");
			failure = 1;
		} else {
			if (EINVAL == errno) {
				printf("errno correctly set\n");
			} else {
				printf("errno not correctly set\n");
				failure = 1;
			}
		}
	}

	if (failure) {
		printf("At least one test FAILED -- see above\n");
		return PTS_FAIL;
	} else {
		printf("All tests PASSED\n");
		return PTS_PASS;
	}
}
