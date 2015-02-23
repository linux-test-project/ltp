/*
 * Copyright (c) 2002-3, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 Test that the difftime function shall return the difference between
 two calendar times.
 */

#define WAIT_DURATION 1

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "posixtest.h"

int main(void)
{
	time_t time0;
	double time_diff;

	time0 = time(NULL);
	time_diff = difftime(time0 + 1, time0);

	if (time_diff != WAIT_DURATION) {
		perror
		    ("Test FAILED: difftime did not return the correct value\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
