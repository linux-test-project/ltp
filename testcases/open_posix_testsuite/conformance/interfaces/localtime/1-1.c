/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 *
 * This test shall return a pointer to the tm structure when converting
 * a time value to a broken down local time.
 */

#include <stdio.h>
#include <time.h>
#include "posixtest.h"

int main(void)
{
	time_t current_time;
	struct tm *timeptr;

	current_time = time(NULL);
	timeptr = NULL;
	timeptr = localtime(&current_time);

	if (timeptr != NULL) {
		printf("date: %s", (asctime(localtime((&current_time)))));
		puts("Test PASSED");
		return PTS_PASS;
	} else {
		puts("Test FAILED: localtime failed");
		return PTS_FAIL;
	}
}
