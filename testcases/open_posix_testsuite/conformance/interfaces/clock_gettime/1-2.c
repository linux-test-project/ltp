/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

   General test that clock_gettime() returns a valid tp for a given
   clock_id (the clock_id chosen for this test is CLOCK_REALTIME).
   Validity is compared with gettimeofday().  See rationale.txt for
   more info.
   If the clocks are within ACCEPTABLEDELTA of each other, the test is
   a pass.
 */
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include "posixtest.h"

#define ACCEPTABLEDELTA 1

int main(void)
{
	struct timespec tpundertest;
	struct timeval tvstandard;
	int delta;

	if (clock_gettime(CLOCK_REALTIME, &tpundertest) == 0) {
		if (gettimeofday(&tvstandard, NULL) == 0) {
			delta = (int)tvstandard.tv_sec -
			    (int)tpundertest.tv_sec;
			if (abs(delta) <= ACCEPTABLEDELTA) {
				printf("Test PASSED\n");
				return PTS_PASS;
			} else {
				printf("FAIL:  expected %d, received %d\n",
				       (int)tvstandard.tv_sec,
				       (int)tpundertest.tv_sec);
				return PTS_FAIL;
			}
		} else {
			perror("Error calling gettimeofday()\n");
			return PTS_UNRESOLVED;
		}
	}

	printf("clock_gettime() failed\n");
	return PTS_UNRESOLVED;
}
