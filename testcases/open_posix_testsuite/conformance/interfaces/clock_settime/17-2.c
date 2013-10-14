/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Error condition API test for the clock_id parameter of the
 * clock_settime() function.
 *
 * Test calling clock_settime() with the following clock_id values:
 *   MIN INT = INT32_MIN
 *   MAX INT = INT32_MAX
 *   MIN INT - 1 = 2147483647  (this is what gcc will set to)
 *   MAX INT + 1 = -2147483647 (this is what gcc will set to)
 *   unassigned value = -1073743192 (ex. of what gcc will set to)
 *   unassigned value = 1073743192 (ex. of what gcc will set to)
 *   -1
 *   17 (currently not = to any clock)
 *
 * The date chosen is Nov 12, 2002 ~11:13am (date when test was first
 * written).
 */
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include "posixtest.h"

#define TESTTIME 1037128358

#define NUMINVALIDTESTS 8

static int invalid_tests[NUMINVALIDTESTS] = {
	INT32_MIN, INT32_MAX, 2147483647, -2147483647, -1073743192,
	1073743192, -1, 17
};

int main(void)
{
	struct timespec tpset;
	int i;
	int failure = 0;

	tpset.tv_sec = TESTTIME;
	tpset.tv_nsec = 0;

	if (geteuid() != 0) {
		printf("Test must be run as superuser\n");
		return PTS_UNRESOLVED;
	}

	for (i = 0; i < NUMINVALIDTESTS; i++) {
		if (clock_settime(invalid_tests[i], &tpset) == -1) {
			if (EINVAL != errno) {
				printf("errno != EINVAL\n");
				failure = 1;
			}
		} else {
			printf("clock_settime() did not return -1\n");
			failure = 1;
		}
	}

	if (failure) {
		printf("At least one test FAILED -- see above\n");
		return PTS_FAIL;
	}

	printf("All tests PASSED\n");
	return PTS_PASS;
}
