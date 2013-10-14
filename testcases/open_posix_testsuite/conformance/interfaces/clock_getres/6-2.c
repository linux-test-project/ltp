/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that clock_getres() sets errno=EINVAL for a variety of
 * invalid clock_ids.
 *
 * The following invalid clock_ids will be used:
 * - Boundary values for integers (TBD if clock_id is an integer)
 *   MIN INT = INT32_MIN
 *   MAX INT = INT32_MAX
 *   MIN INT - 1 = 2147483647  (this is what gcc will set to)
 *   MAX INT + 1 = -2147483647 (this is what gcc will set to)
 *   unassigned value = -1073743192 (ex. of what gcc will set to)
 *   unassigned value = 1073743192 (ex. of what gcc will set to)
 *   -1
 *   17 (currently not = to any clock)
 *
 */
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>
#include "posixtest.h"

#define NUMINVALIDTESTS 8

int main(void)
{
	struct timespec res;
	int invalid_tests[NUMINVALIDTESTS] = {
		INT32_MIN, INT32_MAX, 2147483647, -2147483647, -1073743192,
		1073743192, -1, 17
	};
	int i;
	int failure = 0;

	for (i = 0; i < NUMINVALIDTESTS; i++) {
		printf("clock_getres(%d, &res);\n", invalid_tests[i]);
		if (clock_getres(invalid_tests[i], &res) == -1) {
			if (EINVAL != errno) {
				printf("errno != EINVAL\n");
				failure = 1;
			}
		} else {
			printf("clock_getres() != -1\n");
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
