/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that clock_nanosleep() causes the current thread to be suspended
 * until the time interval in rqtp passes if TIMER_ABSTIME is not set
 * in flags.
 */
#include <stdio.h>
#include <time.h>
#include "posixtest.h"

#define SLEEPNSEC 3000000

int main(void)
{
	struct timespec tssleep, tsbefore, tsafter;
	int slepts = 0, sleptns = 0;

	if (clock_gettime(CLOCK_REALTIME, &tsbefore) != 0) {
		perror("clock_gettime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	tssleep.tv_sec = 0;
	tssleep.tv_nsec = SLEEPNSEC;
	if (clock_nanosleep(CLOCK_REALTIME, 0, &tssleep, NULL) != 0) {
		printf("clock_nanosleep() did not return success\n");
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

	if ((slepts > 0) || (sleptns > SLEEPNSEC)) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("clock_nanosleep() did not sleep long enough\n");
	return PTS_FAIL;
}
