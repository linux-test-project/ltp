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
 * sched_ss_repl_period is not greater than or equal to the
 * sched_ss_init_budget member.
 *
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
	int policy, result;
	int old_priority, old_policy, new_policy;
	struct sched_param param;

	if (sched_getparam(getpid(), &param) != 0) {
		perror("An error occurs when calling sched_getparam()");
		return PTS_UNRESOLVED;
	}
	old_priority = param.sched_priority;

	old_policy = sched_getscheduler(getpid());
	if (old_policy == -1) {
		perror("An error occurs when calling sched_getscheduler()");
		return PTS_UNRESOLVED;
	}

	/* set a sched_ss_repl_period lower than the sched_ss_init_budget */
	param.sched_ss_repl_period.tv_sec = 1;
	param.sched_ss_repl_period.tv_nsec = 0;

	param.sched_ss_init_budget.tv_sec = 2;
	param.sched_ss_init_budget.tv_nsec = 0;

	param.sched_priority = sched_get_priority_max(SCHED_SPORADIC);

	result = sched_setscheduler(0, SCHED_SPORADIC, &param);

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
	}
	if (new_policy != old_policy) {
		printf("The policy has changed\n");
	}
	return PTS_FAIL;

}
#else
int main(void)
{
	printf("Does not support SS (SPORADIC SERVER)\n");
	return PTS_UNSUPPORTED;
}
#endif
