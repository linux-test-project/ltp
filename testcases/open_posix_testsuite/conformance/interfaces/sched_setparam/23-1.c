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
 * Test that the priority remain unchanged when the sched_priority
 * member is not within the inclusive priority range for the current
 * scheduling policy.
 */
#include <sched.h>
#include <stdio.h>
#include "posixtest.h"

int main(void)
{
	int policy, invalid_priority, old_priority;
	struct sched_param param;

	if (sched_getparam(0, &param) != 0) {
		perror("An error occurs when calling sched_getparam()");
		return PTS_UNRESOLVED;
	}
	old_priority = param.sched_priority;

	policy = sched_getscheduler(0);
	if (policy == -1) {
		perror("An error occurs when calling sched_getscheduler()");
		return PTS_UNRESOLVED;
	}

	invalid_priority = sched_get_priority_max(policy);
	if (invalid_priority == -1) {
		perror("An error occurs when calling sched_get_priority_max()");
		return PTS_UNRESOLVED;
	}

	/* set an invalid priority */
	invalid_priority++;
	param.sched_priority = invalid_priority;
	sched_setparam(0, &param);

	if (sched_getparam(0, &param) != 0) {
		perror("An error occurs when calling sched_getparam()");
		return PTS_UNRESOLVED;
	}

	if (param.sched_priority == old_priority) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else {
		printf("The priority have changed.\n");
		return PTS_FAIL;
	}

}
