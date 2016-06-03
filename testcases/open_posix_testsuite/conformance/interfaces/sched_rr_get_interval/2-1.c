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
 * sched_rr_get_interval() returns 0 on success.
 */
#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "posixtest.h"

int main(void)
{
	struct timespec interval;
	int result = -2;
	struct sched_param param;

	param.sched_priority = sched_get_priority_min(SCHED_RR);
	if (sched_setscheduler(0, SCHED_RR, &param) == -1) {
		printf("sched_setscheduler failed: %d (%s)\n",
			errno, strerror(errno));
		return PTS_UNRESOLVED;
	}

	interval.tv_sec = -1;
	interval.tv_nsec = -1;

	result = sched_rr_get_interval(0, &interval);

	if (result == 0 &&
	    interval.tv_sec >= 0 && interval.tv_nsec >= 0 && errno == 0) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	if (interval.tv_sec == -1) {
		printf("interval.tv_sec  not updated.\n");
		return PTS_FAIL;
	}

	if (interval.tv_nsec == -1) {
		printf("interval.tv_nsec  not updated.\n");
		return PTS_FAIL;
	}

	if (result != 0) {
		printf("Returned code != 0.\n");
		return PTS_FAIL;
	}

	if (errno != 0) {
		perror("Unexpected error");
		return PTS_FAIL;
	} else {
		perror("Unresolved test error");
		return PTS_UNRESOLVED;
	}

}
