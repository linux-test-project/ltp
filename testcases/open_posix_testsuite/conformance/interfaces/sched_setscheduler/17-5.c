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
 * Test that the policy and scheduling parameters remain unchanged when the
 * policy value is not defined in the sched.h header.
 *
 * The test attempt to set the policy to a very improbable value.
 * Steps:
 *   1. Get the old policy and priority.
 *   2. Call sched_setscheduler with invalid args.
 *   3. Check that the policy and priority have not changed.
 */
#include <sched.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

int main() {
	int old_priority, old_policy, new_policy;
	struct sched_param param;
	int invalid_policy;

	invalid_policy = 0;
	/* Linux does not treat minus value as invalid for policy */
	while (invalid_policy == SCHED_OTHER ||
		invalid_policy == SCHED_FIFO ||
		invalid_policy == SCHED_RR)
	invalid_policy++;

	if (sched_getparam(getpid(), &param) == -1) {
		perror("An error occurs when calling sched_getparam()");
		return PTS_UNRESOLVED;
	}
	old_priority = param.sched_priority;

	old_policy = sched_getscheduler(getpid());
	if (old_policy == -1) {
		perror("An error occurs when calling sched_getscheduler()");
		return PTS_UNRESOLVED;
	}

	sched_setscheduler(0, invalid_policy, &param);

	if (errno == 0) {
		printf("No error occurs, could %i be a valid value for the scheduling policy ???\n", invalid_policy);
		return PTS_UNRESOLVED;
	}

	if (sched_getparam(getpid(), &param) != 0) {
		perror("An error occurs when calling sched_getparam()");
		return PTS_UNRESOLVED;
	}

	new_policy = sched_getscheduler(getpid());
	if (new_policy == -1) {
		perror("An error occurs when calling sched_getscheduler()");
		return PTS_UNRESOLVED;
	}

	if (old_policy == new_policy &&
	   old_priority == param.sched_priority) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	if (param.sched_priority != old_priority) {
		printf("The param has changed\n");
	}
	if (new_policy != old_policy) {
		printf("The policy has changed\n");
	}
	return PTS_FAIL;
}