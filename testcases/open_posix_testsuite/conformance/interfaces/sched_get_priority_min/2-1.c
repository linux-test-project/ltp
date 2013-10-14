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
 * Test that sched_get_priority_min() returns -1 on failure
 */
#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{
	int result = -1;

	result = sched_get_priority_min(-1);

	if (result == -1 && errno == EINVAL) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	if (result != -1) {
		printf("did not returned -1.\n");
		return PTS_FAIL;
	}

	if (errno != EINVAL) {
		perror("error is not EINVAL");
		return PTS_FAIL;
	}

	printf("Unresolved test error\n");
	return PTS_UNRESOLVED;
}
