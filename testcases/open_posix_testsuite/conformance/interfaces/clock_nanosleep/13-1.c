/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that clock_nanosleep() sets errno to EINVAL if clock_id does
 * not refer to a known clock.
 */
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include "posixtest.h"

#define SLEEPSEC 4

#define BOGUSCLOCKID 99999

int main(int argc, char *argv[])
{
	struct timespec tssleep;

	tssleep.tv_sec=SLEEPSEC;
	tssleep.tv_nsec=0;

	if (clock_nanosleep(BOGUSCLOCKID, 0, &tssleep, NULL) == EINVAL) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else {
		printf("errno != EINVAL\n");
		return PTS_FAIL;
	}

	printf("This code should not be executed\n");
	return PTS_UNRESOLVED;
}
