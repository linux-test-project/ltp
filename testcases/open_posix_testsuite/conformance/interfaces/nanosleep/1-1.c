/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that nanosleep() causes the current thread to be suspended
 * until the time interval in rqtp passes.
 */
#include <stdio.h>
#include <time.h>
#include "posixtest.h"

int main(void)
{
	struct timespec tssleepfor, tsstorage, tsbefore, tsafter;
	int sleepnsec = 3;
	int slepts = 0, sleptns = 0;

	if (clock_gettime(CLOCK_REALTIME, &tsbefore) == -1) {
		perror("Error in clock_gettime()\n");
		return PTS_UNRESOLVED;
	}

	tssleepfor.tv_sec = 0;
	tssleepfor.tv_nsec = sleepnsec;
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

	if ((slepts > 0) || (sleptns > sleepnsec)) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("nanosleep() did not sleep long enough\n");
	return PTS_FAIL;
}
