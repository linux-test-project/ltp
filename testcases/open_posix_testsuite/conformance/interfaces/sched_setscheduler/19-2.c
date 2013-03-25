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
 * Test that sched_setscheduler() sets errno == EINVAL when the
 * sched_ss_low_priority member is not within the inclusive priority range for
 * the sporadic server policy.
 *
 * @pt:SS
 */

#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include "posixtest.h"

#if defined(_POSIX_SPORADIC_SERVER)&&(_POSIX_SPORADIC_SERVER != -1)

int main(void)
{
	int invalid_priority, result;
	struct sched_param param;

	invalid_priority = sched_get_priority_max(SCHED_SPORADIC);
	if (invalid_priority == -1) {
		perror("An error occurs when calling sched_get_priority_max()");
		return PTS_UNRESOLVED;
	}

	/* set an invalid priority */
	invalid_priority++;
	param.sched_ss_low_priority = invalid_priority;

	result = sched_setscheduler(0, SCHED_SPORADIC, &param);

	if (result == -1 && errno == EINVAL) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (result != -1) {
		printf("The returned code is not -1.\n");
		return PTS_FAIL;
	} else if (errno == EPERM) {
		printf
		    ("This process does not have the permission to set its own scheduling policy.\nTry to launch this test as root.\n");
		return PTS_UNRESOLVED;
	} else {
		perror("Unknow error");
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
