/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test nanosleep() on a variety of valid and invalid input parameters.
 *
 * For valid parameters, if the seconds spent is within OKSECERR, the
 * test is considered a pass (Note:  This is not too accurate since
 * accuracy is at the second level.).
 *
 * For invalid parameters, nanosleep should fail with -1 exit and
 * errno set to EINVAL.
 */
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include "posixtest.h"

#define NUMVALID 6
#define NUMINVALID 7

#define OKSECERR 1

/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * input tests
 */
static int sleepvalid[NUMVALID][2] = { {0, 30000000}, {1, 0},
{1, 30000000}, {2, 0},
{10, 5000}, {13, 5}
};
static int sleepinvalid[NUMINVALID][2] = { {-1, -1}, {0, -1},
{1, 1000000000}, {2, 1000000000},
{-2147483647, -2147483647},
{1, 2147483647},
{0, 1075002478}
};

int main(void)
{
	struct timespec tssleepfor, tsstorage, tsbefore, tsafter;
	int i;
	int failure = 0;
	int slepts = 0, sleptns = 0;

	for (i = 0; i < NUMVALID; i++) {
		tssleepfor.tv_sec = sleepvalid[i][0];
		tssleepfor.tv_nsec = sleepvalid[i][1];
		printf("sleep %d sec %d nsec\n",
		       (int)tssleepfor.tv_sec, (int)tssleepfor.tv_nsec);
		if (clock_gettime(CLOCK_REALTIME, &tsbefore) == -1) {
			perror("Error in clock_gettime()\n");
			return PTS_UNRESOLVED;
		}

		if (nanosleep(&tssleepfor, &tsstorage) == 0) {
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

			if ((slepts - tssleepfor.tv_sec) > OKSECERR) {
				printf("FAIL - slept %ds%dns >> %lds%ldns\n",
				       slepts, sleptns,
				       tssleepfor.tv_sec, tssleepfor.tv_nsec);
				failure = 1;
			} else {
				printf("PASS - slept %ds%dns ~= %lds%ldns\n",
				       slepts, sleptns,
				       tssleepfor.tv_sec, tssleepfor.tv_nsec);
			}
		} else {
			printf("nanosleep() did not return 0 on success\n");
			failure = 1;
		}
	}

	for (i = 0; i < NUMINVALID; i++) {
		tssleepfor.tv_sec = sleepinvalid[i][0];
		tssleepfor.tv_nsec = sleepinvalid[i][1];
		printf("sleep %d sec %d nsec\n",
		       (int)tssleepfor.tv_sec, (int)tssleepfor.tv_nsec);
		if (nanosleep(&tssleepfor, &tsstorage) == -1) {
			if (EINVAL != errno) {
				printf("errno != EINVAL\n");
				failure = 1;
			}
		} else {
			printf("nanosleep() did not return -1 on failure\n");
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
	return PTS_PASS;
}
