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
 * Test that sched_getscheduler() returns the scheduling policy of the
 * specified process on success.
 */
#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{
	int result = -1;

	result = sched_getscheduler(0);

	if (result != -1 && errno == 0) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	if (errno != 0) {
		perror("Unexpected error");
		return PTS_FAIL;
	} else {
		perror("Unresolved test error");
		return PTS_UNRESOLVED;
	}

}
