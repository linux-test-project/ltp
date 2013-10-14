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
 * Test that sched_get_priority_min() returns the appropriate minimum value on
 * success for SCHED_FIFO policy.
 */
#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{
	int result = -1;

	result = sched_get_priority_min(SCHED_FIFO);

	if (result != -1 && errno == 0) {
		printf("The minimum priority for policy SCHED_FIFO is %i.\n",
		       result);
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	perror("An error occurs");
	return PTS_FAIL;
}
