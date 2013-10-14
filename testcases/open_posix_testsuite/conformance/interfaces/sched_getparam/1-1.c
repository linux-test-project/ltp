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
 * Test that sched_getparam() function return the scheduling parameters of a
 * process specified by pid in the sched_param structure pointed to by param.
 */
#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

int main(void)
{
	struct sched_param param;
	int result = -1;

	param.sched_priority = -1;

	result = sched_getparam(getpid(), &param);

	if (result == 0 && param.sched_priority != -1 && errno == 0) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	if (result != 0) {
		printf("Return code is not zero.\n");
		return PTS_FAIL;
	}

	if (errno != 0) {
		perror("Unexpected error");
		return PTS_FAIL;
	}

	if (param.sched_priority == -1) {
		printf("The 'sched_priority' member does not change.\n");
		return PTS_FAIL;
	}

	printf("This code should not be executed.\n");
	return PTS_UNRESOLVED;
}
