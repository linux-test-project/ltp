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
 * Test that sched_get_priority_max() returns the maximum value on
 * success for SCHED_SPORADIC policy.
 */
#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#if defined(_POSIX_SPORADIC_SERVER)&&(_POSIX_SPORADIC_SERVER != -1)||defined(_POSIX_THREAD_SPORADIC_SERVER)&&(_POSIX_THREAD_SPORADIC_SERVER != -1)

int main(void)
{
	int result = -1;

	result = sched_get_priority_max(SCHED_SPORADIC);

	if (result != -1 && errno == 0) {
		printf
		    ("The maximum priority for policy SCHED_SPORADIC is %i.\n",
		     result);
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	perror("An error occurs");
	return PTS_FAIL;
}
#else
int main(void)
{
	printf("Does not support SS (SPORADIC SERVER)\n");
	return PTS_UNSUPPORTED;
}
#endif
