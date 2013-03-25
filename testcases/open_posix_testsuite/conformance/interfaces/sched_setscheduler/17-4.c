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
 * Test that the policy and scheduling parameters remain unchanged when
 * the sched_ss_max_repl is not within the inclusive range [1,SS_REPL_MAX]
 *
 * Test with a sched_ss_max_repl value of 0.
 * Steps:
 *   1. Get the old policy and priority.
 *   2. Call sched_setscheduler with invalid args.
 *   3. Check that the policy and priority have not changed.
 *
 * @pt:SS
 */

#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include <unistd.h>
#include "posixtest.h"

#if defined(_POSIX_SPORADIC_SERVER)&&(_POSIX_SPORADIC_SERVER != -1)

int main(void)
{
	int max_priority, old_priority, old_policy, new_policy;
	struct sched_param param;

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

	/* Make sure that param.sched_priority != old_priority */
	max_priority = sched_get_priority_max(SCHED_SPORADIC);
	param.sched_priority = (old_priority == max_priority) ?
	    sched_get_priority_min(SCHED_SPORADIC) : max_priority;

	param.sched_ss_max_repl = 0;

	sched_setscheduler(0, SCHED_SPORADIC, &param);

	if (sched_getparam(getpid(), &param) != 0) {
		perror("An error occurs when calling sched_getparam()");
		return PTS_UNRESOLVED;
	}

	new_policy = sched_getscheduler(getpid());
	if (new_policy == -1) {
		perror("An error occurs when calling sched_getscheduler()");
		return PTS_UNRESOLVED;
	}

	if (old_policy == new_policy && old_priority == param.sched_priority) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	if (param.sched_priority != old_priority) {
		printf("The param has changed\n");
		return PTS_FAIL;
	}
	if (new_policy != old_policy) {
		printf("The policy has changed\n");
		return PTS_FAIL;
	}

}
#else
int main(void)
{
	printf("Does not support SS (SPORADIC SERVER)\n");
	return PTS_UNSUPPORTED;
}
#endif
