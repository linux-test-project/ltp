/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that clock_settime() sets errno=EINVAL if tp has a nsec value < 0 or
 * >= 1000 million.
 *
 * Test calling clock_settime() with the following tp.tv_nsec values:
 *   MIN INT = INT32_MIN
 *   MAX INT = INT32_MAX
 *   MIN INT - 1 = 2147483647  (this is what gcc will set to)
 *   MAX INT + 1 = -2147483647 (this is what gcc will set to)
 *   unassigned value = -1073743192 (ex. of what gcc will set to)
 *   unassigned value = 1073743192 (ex. of what gcc will set to)
 *   -1
 *   1000000000
 *   1000000001
 *
 * The clock_id CLOCK_REALTIME is used.
 */
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include "posixtest.h"
#include "helpers.h"

#define NUMINVALIDTESTS 9

static int invalid_tests[NUMINVALIDTESTS] = {
	INT32_MIN, INT32_MAX, 2147483647, -2147483647, -1073743192,
	1073743192, -1, 1000000000, 1000000001
};

int main(void)
{
	struct timespec tsset, tscurrent, tsreset;
	int i;
	int failure = 0;

	/* Check that we're root...can't call clock_settime with CLOCK_REALTIME otherwise */
	if (getuid() != 0) {
		printf("Run this test as ROOT, not as a Regular User\n");
		return PTS_UNTESTED;
	}

	if (clock_gettime(CLOCK_REALTIME, &tscurrent) != 0) {
		printf("clock_gettime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	getBeforeTime(&tsreset);
	for (i = 0; i < NUMINVALIDTESTS; i++) {
		tsset.tv_sec = tscurrent.tv_sec;
		tsset.tv_nsec = invalid_tests[i];

		printf("Test %d sec %d nsec\n",
		       (int)tsset.tv_sec, (int)tsset.tv_nsec);
		if (clock_settime(CLOCK_REALTIME, &tsset) == -1) {
			if (EINVAL != errno) {
				printf("errno != EINVAL\n");
				failure = 1;
			}
		} else {
			printf("clock_settime() did not return -1\n");
			failure = 1;
		}
	}

	setBackTime(tsreset);
	if (failure) {
		printf("At least one test FAILED -- see above\n");
		return PTS_FAIL;
	} else {
		printf("All tests PASSED\n");
		return PTS_PASS;
	}

	return PTS_UNRESOLVED;
}
