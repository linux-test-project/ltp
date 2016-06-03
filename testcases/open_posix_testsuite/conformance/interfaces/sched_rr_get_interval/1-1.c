/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *
 * Test that the current execution time limit is returned for the calling
 * process when pid = 0.
 */
#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "posixtest.h"

int main(void)
{

	struct timespec interval0;
	struct timespec interval1;
	int result0 = -1;
	int result1 = -1;
	struct sched_param param;

	param.sched_priority = sched_get_priority_min(SCHED_RR);
	if (sched_setscheduler(0, SCHED_RR, &param) == -1) {
		printf("sched_setscheduler failed: %d (%s)\n",
			errno, strerror(errno));
		return PTS_UNRESOLVED;
	}

	interval0.tv_sec = -1;
	interval0.tv_nsec = -1;

	interval1.tv_sec = -1;
	interval1.tv_nsec = -1;

	result0 = sched_rr_get_interval(0, &interval0);
	result1 = sched_rr_get_interval(getpid(), &interval1);

	if (result0 == result1 &&
	    interval0.tv_sec == interval1.tv_sec &&
	    interval0.tv_nsec == interval1.tv_nsec && errno == 0) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (errno != 0) {
		perror("Unexpected error");
		return PTS_FAIL;
	} else {
		printf
		    ("Different results between pid == 0 and pid == getpid().\n");
		return PTS_FAIL;
	}

}
