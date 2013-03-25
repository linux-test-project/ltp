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
 * Test that the function return the former scheduling policy of the specified
 * process upon successful completion.
 *
 * Steps:
 *   1. Get the old policy.
 *   2. Set a new policy.
 *   3. Check that sched_setscheduler return the old policy.
 */
#include <sched.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

int main(void)
{
	int result, old_policy, new_policy;
	struct sched_param param;

	old_policy = sched_getscheduler(getpid());
	if (old_policy == -1) {
		perror("An error occurs when calling sched_getscheduler()");
		return PTS_UNRESOLVED;
	}

	/* Make sure new_policy != old_policy */
	new_policy = (old_policy == SCHED_FIFO) ? SCHED_RR : SCHED_FIFO;

	param.sched_priority = sched_get_priority_max(new_policy);
	result = sched_setscheduler(0, new_policy, &param);

	if (result == old_policy) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (result == -1 && errno == EPERM) {
		printf
		    ("The process have not permission to change its own policy.\nTry to launch this test as root.\n");
		return PTS_UNRESOLVED;
	}

	printf("Returned code == %i.\n", result);
	perror("Unknow error");
	return PTS_FAIL;

}
