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
 * Test that the scheduling parameters are returned for the calling process
 * when pid = 0.
 */
#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

int main(void)
{

	struct sched_param param0;
	struct sched_param param1;
	int result0;
	int result1;

	param0.sched_priority = -1;
	param1.sched_priority = -1;

	result0 = sched_getparam(0, &param0);
	result1 = sched_getparam(getpid(), &param1);

	if (result0 == result1 &&
	    param0.sched_priority == param1.sched_priority && errno == 0) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("Different results between pid == 0 "
	       "and pid == getpid().\n");
	return PTS_FAIL;
}
