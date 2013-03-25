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
 * Test that the sched_setparam() function return zero on success.
 */
#include <sched.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{
	int result;
	struct sched_param param;

	if (sched_getparam(0, &param) == -1) {
		perror("An error occurs when calling sched_getparam()");
		return PTS_UNRESOLVED;
	}

	result = sched_setparam(0, &param);

	if (result == 0) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (errno == EPERM) {
		printf
		    ("This process does not have the permission to set its own scheduling parameter.\nTry to launch this test as root.\n");
		return PTS_UNRESOLVED;
	} else {
		printf("returned code is not 0.\n");
		return PTS_FAIL;
	}
}
