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
 * Test that the scheduling policy is returned for the calling process when
 * pid = 0
 */
#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

int main(void)
{

	int result0 = -1;
	int result1 = -1;

	result0 = sched_getscheduler(0);
	result1 = sched_getscheduler(getpid());

	if (result0 == result1 && errno == 0) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	if (result0 != result1) {
		printf("Different results between pid == 0 "
		       "and pid == getpid().\n");
		return PTS_FAIL;
	}

	perror("Unexpected error");
	return PTS_FAIL;
}
