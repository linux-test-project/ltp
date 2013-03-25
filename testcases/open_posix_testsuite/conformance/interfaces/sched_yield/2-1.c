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
 * Test that sched_yield() return 0 upon success.
 */
#include <sched.h>
#include <stdio.h>
#include "posixtest.h"

int main(void)
{
	if (sched_yield() == 0) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("sched_yield() does not return 0.\n");
	return PTS_FAIL;
}
