/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that clock_nanosleep() sets errno to EINVAL if rqtp contained a
 * nanosecond value < 0 or >= 1,000 million
 */
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>
#include "posixtest.h"

#define NUMINVALIDTESTS 9

static int invalid_tests[NUMINVALIDTESTS] = {
	INT32_MIN,
	INT32_MAX,
	2147483647,
	-2147483647,
	-1073743192,
	1073743192,
	-1,
	1000000000,
	1000000001
};

int main(void)
{
	struct timespec tssleep;
	int i;
	int failure = 0;

	tssleep.tv_sec = 0;

	for (i = 0; i < NUMINVALIDTESTS; i++) {
		tssleep.tv_nsec = invalid_tests[i];
		printf("sleep %d\n", invalid_tests[i]);
		if (clock_nanosleep(CLOCK_REALTIME, 0, &tssleep, NULL) !=
		    EINVAL) {
			printf("errno != EINVAL\n");
			failure = 1;
		}
	}

	if (failure) {
		printf("At least one test FAILED\n");
		return PTS_FAIL;
	} else {
		printf("All tests PASSED\n");
		return PTS_PASS;
	}
}
