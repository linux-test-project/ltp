/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that timer_settime() sets errno = EINVAL if:
 * - value.it_value is not = 0 (so disabled)
 * AND
 * - value.*.it_nsec < 0
 * or
 * - value.*.it_nsec >= 1000 million
 * This includes standard boundary tests.
 *
 * Signal SIGTOTEST is used.
 * Clock CLOCK_REALTIME is used.
 */

#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

#define SIGTOTEST SIGALRM
#define NUMTESTS 20

/*
 * Each test listed in the multidimensional array.
 * element 0 - value.it_value.tv_sec
 * element 1 - value.it_value.tv_nsec
 * element 2 - value.it_interval.tv_sec
 * element 3 - value.it_interval.tv_nsec
 */
static int testlist[NUMTESTS][4] = {
	{1, -1, 0, 0},		// value.it_value.tv_nsec < 0
	{1, -2147483647, 0, 0},	// value.it_value.tv_nsec < 0
	{1, -1073743192, 0, 0},	// value.it_value.tv_nsec < 0
	{1, 1000000000, 0, 0},	// value.it_value.tv_nsec >= 1K mi
	{1, 1000000001, 0, 0},	// value.it_value.tv_nsec >= 1K mi
	{1, 2147483647, 0, 0},	// value.it_value.tv_nsec >= 1K mi
	{1, 1075002478, 0, 0},	// value.it_value.tv_nsec >= 1K mi
	{1, -1, 0, -1},		// value.it_interval.tv_nsec < 0
	{1, 0, 0, -2147483647},	// value.it_interval.tv_nsec < 0
	{1, 0, 0, -1073743192},	// value.it_interval.tv_nsec < 0
	{1, 0, 0, 1000000000},	// value.it_interval.tv_nsec >= 1K mi
	{1, 0, 0, 1000000001},	// value.it_interval.tv_nsec >= 1K mi
	{1, 0, 0, 2147483647},	// value.it_interval.tv_nsec >= 1K mi
	{1, 0, 0, 1075002478},	// value.it_interval.tv_nsec >= 1K mi
	{-1, 0, 0, 0},		// value.it_value.tv_sec < 0
	{-2147483647, 0, 0, 0},	// value.it_value.tv_sec < 0
	{-1073743192, 0, 0, 0},	// value.it_value.tv_sec < 0
	{1, 0, -1, 0},		// value.it_interval.tv_sec < 0
	{1, 0, -2147483647, 0},	// value.it_interval.tv_sec < 0
	{1, 0, -1073743192, 0},	// value.it_interval.tv_sec < 0
};

int main(void)
{
	struct sigevent ev;
	timer_t tid;
	struct itimerspec its;
	int i;
	int failure = 0;

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGTOTEST;

	if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
		perror("timer_create() did not return success\n");
		return PTS_UNRESOLVED;
	}

	for (i = 0; i < NUMTESTS; i++) {
		its.it_value.tv_sec = testlist[i][0];
		its.it_value.tv_nsec = testlist[i][1];
		its.it_interval.tv_sec = testlist[i][2];
		its.it_interval.tv_nsec = testlist[i][3];

		printf("it_value: %d sec %d nsec\n",
		       (int)its.it_value.tv_sec, (int)its.it_value.tv_nsec);
		printf("it_interval: %d sec %d nsec\n",
		       (int)its.it_interval.tv_sec,
		       (int)its.it_interval.tv_nsec);

		if (timer_settime(tid, 0, &its, NULL) == -1) {
			if (EINVAL != errno) {
				printf("errno != EINVAL\n");
				failure = 1;
			}
		} else {
			printf("timer_settime() did not return failure\n");
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
