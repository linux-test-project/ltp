/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 *
 * This test case checks if the return value of the ctime call is
 * not NULL after converting the time value to a date and time string.
 */

#include <stdio.h>
#include <time.h>
#include "posixtest.h"

int main(void)
{
	time_t current_time;
	char *result;

	time(&current_time);
	result = ctime(&current_time);

	if (result == NULL) {
		puts("Test FAILED: returned NULL");
		return PTS_FAIL;
	} else {
		printf("converted date and time is: %s\n",
		       ctime(&current_time));
		printf("Test PASSED\n");
		return PTS_PASS;
	}
}
