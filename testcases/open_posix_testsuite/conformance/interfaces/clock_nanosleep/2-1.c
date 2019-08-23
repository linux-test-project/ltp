/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that if TIMER_ABSTIME is set in flags, then clock_nanosleep()
 * sleeps for the absolute time specified in rqtp.
 *
 * - Get the current time.
 * - Set clock_nanosleep() to sleep for current time + SLEEPSEC
 *   seconds.
 * - Get the current time after sleeping and ensure it is within
 *   ACCEPTABLEDELTA of current time + SLEEPSEC
 */
#include <stdio.h>
#include <time.h>
#include "posixtest.h"

#define SLEEPSEC 3
#define ACCEPTABLEDELTA 1

int main(void)
{
	struct timespec tssleep, tsbefore, tsafter;
	time_t sleepuntilsec;
	int flags = 0;

	if (clock_gettime(CLOCK_REALTIME, &tsbefore) != 0) {
		perror("clock_gettime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	sleepuntilsec = tsbefore.tv_sec + SLEEPSEC;
	tssleep.tv_sec = sleepuntilsec;
	tssleep.tv_nsec = tsbefore.tv_nsec;

	flags |= TIMER_ABSTIME;
	if (clock_nanosleep(CLOCK_REALTIME, flags, &tssleep, NULL) != 0) {
		printf("clock_nanosleep() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (clock_gettime(CLOCK_REALTIME, &tsafter) == -1) {
		perror("Error in clock_gettime()\n");
		return PTS_UNRESOLVED;
	}

	if (tsafter.tv_sec >= sleepuntilsec) {
		if (tsafter.tv_sec <= (sleepuntilsec + ACCEPTABLEDELTA)) {
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("clock_nanosleep() slept too long\n");
			return PTS_FAIL;
		}
	}

	printf("clock_nanosleep() did not sleep long enough\n");
	return PTS_FAIL;
}
