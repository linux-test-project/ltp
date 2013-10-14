/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that nanosleep() causes the current thread to be suspended
 * until _at least_ the time interval in rqtp passes.
 * Test for a variety of time intervals (in nsecs)
 */
#include <stdio.h>
#include <time.h>
#include "posixtest.h"

#define NUMINTERVALS 13
int main(void)
{
	struct timespec tssleepfor, tsstorage, tsbefore, tsafter;
	int sleepnsec[NUMINTERVALS] = { 1, 2, 10, 100, 1000, 10000, 1000000,
		10000000, 100000000, 200000000, 500000000, 750000000,
		999999900
	};
	int i;
	int failure = 0;
	int slepts, sleptns;

	if (clock_gettime(CLOCK_REALTIME, &tsbefore) == -1) {
		perror("Error in clock_gettime()\n");
		return PTS_UNRESOLVED;
	}

	tssleepfor.tv_sec = 0;
	for (i = 0; i < NUMINTERVALS; i++) {
		tssleepfor.tv_nsec = sleepnsec[i];
		if (nanosleep(&tssleepfor, &tsstorage) != 0) {
			printf("nanosleep() did not return success\n");
			return PTS_UNRESOLVED;
		}

		if (clock_gettime(CLOCK_REALTIME, &tsafter) == -1) {
			perror("Error in clock_gettime()\n");
			return PTS_UNRESOLVED;
		}

		/*
		 * Generic alg for calculating slept time.
		 */
		slepts = tsafter.tv_sec - tsbefore.tv_sec;
		sleptns = tsafter.tv_nsec - tsbefore.tv_nsec;
		if (sleptns < 0) {
			sleptns = sleptns + 1000000000;
			slepts = slepts - 1;
		}

		if (slepts >= 1 || sleptns > sleepnsec[i]) {
			printf("PASS slept %ds %dns >= %d\n",
			       slepts, sleptns, sleepnsec[i]);
		} else {
			printf("FAIL slept %ds %dns < %d\n",
			       slepts, sleptns, sleepnsec[i]);
			failure = 1;
		}
	}

	if (failure) {
		printf("At least one test FAILED\n");
		return PTS_FAIL;
	}

	printf("All tests PASSED\n");
	return PTS_PASS;
}
