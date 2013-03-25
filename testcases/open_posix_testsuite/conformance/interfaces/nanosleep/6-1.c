/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that nanosleep() sets errno to EINVAL if rqtp contained a
 * nanosecond value < 0 or >= 1,000 million
 */
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include "posixtest.h"

#define NUMTESTS 7

int main(void)
{
	struct timespec tssleepfor, tsstorage;
	int sleepnsec[NUMTESTS] = { -1, -5, -1000000000, 1000000000,
		1000000001, 2000000000, 2000000000
	};
	int i;
	int failure = 0;

	tssleepfor.tv_sec = 0;

	for (i = 0; i < NUMTESTS; i++) {
		tssleepfor.tv_nsec = sleepnsec[i];
		printf("sleep %d\n", sleepnsec[i]);
		if (nanosleep(&tssleepfor, &tsstorage) == -1) {
			if (EINVAL != errno) {
				printf("errno != EINVAL\n");
				failure = 1;
			}
		} else {
			printf("nanosleep() did not return -1 on failure\n");
			return PTS_UNRESOLVED;
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
