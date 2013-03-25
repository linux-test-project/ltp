/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 *
 * The gmtime function shall return a pointer to struct tm.
 *
 */

#include <stdio.h>
#include <time.h>
#include <posixtest.h>

int main(void)
{
	struct tm *tm_ptr;
	time_t the_time;

	(void)time(&the_time);
	tm_ptr = NULL;
	tm_ptr = gmtime(&the_time);

	if (tm_ptr != NULL) {
		puts("Test PASSED");
		return PTS_PASS;
	} else {
		puts("Test FAILED");
		return PTS_FAIL;
	}
}
