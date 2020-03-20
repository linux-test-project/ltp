/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that the clock time can be set to a large number, Y2K
 * critical dates, and times around daylight savings.
 *
 * Test for CLOCK_REALTIME.  (N/A for CLOCK_MONOTONIC as that clock
 * cannot be set.)
 */
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include "posixtest.h"

#define NUMTESTS 5

#define ACCEPTABLESECDELTA 0
#define ACCEPTABLENSECDELTA 5000000

static int testtimes[NUMTESTS][2] = {
	{INT32_MAX, 999999999},	// large number
	{946713600, 999999999},	// Y2K - Jan 1, 2000
	{951811200, 999999999},	// Y2K - Feb 29, 2000
	{1078041600, 999999999},	// Y2K - Feb 29, 2004
	{1049623200, 999999999},	// daylight savings 2003
};

int main(int argc, char *argv[])
{
	struct timespec tpset, tpget, tsreset;
	int secdelta, nsecdelta;
	int failure = 0;
	int i;

	if (clock_gettime(CLOCK_REALTIME, &tsreset) != 0) {
		perror("clock_gettime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	for (i = 0; i < NUMTESTS; i++) {
		tpset.tv_sec = testtimes[i][0];
		tpset.tv_nsec = testtimes[i][1];
#ifdef DEBUG
		printf("Test: %ds %dns\n", testtimes[i][0], testtimes[i][1]);
#endif
		if (clock_settime(CLOCK_REALTIME, &tpset) == 0) {
			if (clock_gettime(CLOCK_REALTIME, &tpget) == -1) {
				printf("Error in clock_gettime()\n");
				return PTS_UNRESOLVED;
			}
			secdelta = tpget.tv_sec - tpset.tv_sec;
			nsecdelta = tpget.tv_nsec - tpset.tv_nsec;
			if (nsecdelta < 0) {
				nsecdelta = nsecdelta + 1000000000;
				secdelta = secdelta - 1;
			}
#ifdef DEBUG
			printf("Delta:  %ds %dns\n", secdelta, nsecdelta);
#endif
			if ((secdelta > ACCEPTABLESECDELTA) || (secdelta < 0)) {
				printf("clock does not appear to be set\n");
				failure = 1;
			}
			if ((nsecdelta > ACCEPTABLENSECDELTA) ||
			    (nsecdelta < 0)) {
				printf("clock does not appear to be set\n");
				failure = 1;
			}
		} else {
			printf("clock_settime() failed\n");
			return PTS_UNRESOLVED;
		}

		if (clock_settime(CLOCK_REALTIME, &tsreset) != 0) {
			perror("clock_settime() did not return success\n");
			return PTS_UNRESOLVED;
		}
	}

	if (clock_settime(CLOCK_REALTIME, &tsreset) != 0) {
		perror("clock_settime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (failure) {
		printf("At least one test FAILED\n");
		return PTS_FAIL;
	}

	printf("All tests PASSED\n");
	return PTS_PASS;
}
