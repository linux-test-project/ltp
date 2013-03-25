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
 * Test that sched_setparam() sets errno == EINVAL when the sched_priority
 * member is not within the inclusive priority range for the current
 * scheduling policy.
 */
#include <sched.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{
	int policy, invalid_priority, result;
	struct sched_param param;

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

	result = sched_setparam(0, &param);

	if (result == -1 && errno == EINVAL) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (result != -1) {
		printf("The returned code is not -1.\n");
		return PTS_FAIL;
	} else if (errno == EPERM) {
		printf
		    ("This process does not have the permission to set its own scheduling parameter.\nTry to launch this test as root\n");
		return PTS_UNRESOLVED;
	} else {
		perror("Unknow error");
		return PTS_FAIL;
	}
}
