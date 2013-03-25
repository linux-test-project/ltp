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
	int result, new_priority, old_priority, max_prio;
	int old_policy, new_policy, test_policy;
	struct sched_param param;

	if (sched_getparam(getpid(), &param) != 0) {
		perror("An error occurs when calling sched_getparam()");
		return PTS_UNRESOLVED;
	}

	old_policy = sched_getscheduler(getpid());
	if (old_policy == -1) {
		perror("An error occurs when calling sched_getscheduler()");
		return PTS_UNRESOLVED;
	}

	/* Make sure new_policy != old_policy */
	new_policy = (old_policy == SCHED_FIFO) ? SCHED_RR : SCHED_FIFO;

	/* Make sure new_priority != old_priority */
	max_prio = sched_get_priority_max(new_policy);
	old_priority = param.sched_priority;
	new_priority = (old_priority == max_prio) ?
	    (param.sched_priority = sched_get_priority_min(new_policy)) :
	    (param.sched_priority = max_prio);

	result = sched_setscheduler(0, new_policy, &param);

	if (sched_getparam(getpid(), &param) != 0) {
		perror("An error occurs when calling sched_getparam()");
		return PTS_UNRESOLVED;
	}

	test_policy = sched_getscheduler(getpid());
	if (test_policy == -1) {
		perror("An error occurs when calling sched_getscheduler()");
		return PTS_UNRESOLVED;
	}

	if (result != -1 && param.sched_priority == new_priority &&
	    test_policy == new_policy) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (result != -1 &&
		   (param.sched_priority == old_priority ||
		    test_policy == old_policy)) {
		if (param.sched_priority == old_priority) {
			printf("The param does not change\n");
		}
		if (test_policy == old_policy) {
			printf("The policy does not change\n");
		}
		return PTS_FAIL;
	} else if (result == -1 && errno == EPERM) {
		printf
		    ("The process have not permission to change its own policy.\nTry to launch this test as root.\n");
		return PTS_UNRESOLVED;
	}

	perror("Unknow error");
	return PTS_FAIL;
}
