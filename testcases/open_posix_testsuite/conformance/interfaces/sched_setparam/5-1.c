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
 * Test that the scheduling policy and scheduling parameters are set for
 * the calling process when pid == 0.
 */
#include <sched.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

int main(void)
{
	int result, new_priority, old_priority, max_prio, policy;
	struct sched_param param;

	if (sched_getparam(getpid(), &param) != 0) {
		perror("An error occurs when calling sched_getparam()");
		return PTS_UNRESOLVED;
	}

	policy = sched_getscheduler(getpid());
	if (policy == -1) {
		perror("An error occurs when calling sched_getscheduler()");
		return PTS_UNRESOLVED;
	}

	/* Make sure new_priority != old_priority */
	max_prio = sched_get_priority_max(policy);
	old_priority = param.sched_priority;
	new_priority = (old_priority == max_prio) ?
	    sched_get_priority_min(policy) : max_prio;
	param.sched_priority = new_priority;

	result = sched_setparam(0, &param);

	if (sched_getparam(getpid(), &param) != 0) {
		perror("An error occurs when calling sched_getparam()");
		return PTS_UNRESOLVED;
	}

	if (result == 0 && param.sched_priority == new_priority) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (result == 0 && param.sched_priority == old_priority) {
		printf("The param does not change\n");
		return PTS_FAIL;
	} else if (result == -1 && errno == EPERM) {
		printf
		    ("The process have not permission to change its own param.\n");
		return PTS_UNRESOLVED;
	}

	perror("Unknow error");
	return PTS_FAIL;
}
