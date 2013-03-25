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
 * Test that the priority remain unchanged when the sched_ss_repl_period is not
 * greater than or equal to the sched_ss_init_budget member.
 *
 * @pt:SS
 */
#include <sched.h>
#include <stdio.h>
#include <unistd.h>
#include "posixtest.h"

#if defined(_POSIX_SPORADIC_SERVER)&&(_POSIX_SPORADIC_SERVER != -1)

int main(void)
{
	int old_priority;
	struct sched_param param;

	if (sched_getparam(0, &param) == -1) {
		perror("An error occurs when calling sched_getparam()");
		return PTS_UNRESOLVED;
	}
	old_priority = param.sched_priority;

	/* set a sched_ss_repl_period lower than the sched_ss_init_budget */
	param.sched_ss_repl_period.tv_sec = 1;
	param.sched_ss_repl_period.tv_nsec = 0;

	param.sched_ss_init_budget.tv_sec = 2;
	param.sched_ss_init_budget.tv_nsec = 0;

	param.sched_priority++;
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

#else
int main(void)
{
	printf("Does not support SS (SPORADIC SERVER)\n");
	return PTS_UNSUPPORTED;
}

#endif
