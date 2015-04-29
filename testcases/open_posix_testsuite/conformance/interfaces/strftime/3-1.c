/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 *
 * strftime shall return the number of bytes placed into the array.
 * Otherwise, it should return 0.
 */

#include <time.h>
#include <stdio.h>
#include "posixtest.h"

int main(void)
{
	struct tm *tm_ptr;
	time_t the_time;
	char buf[256];
	int result;

	(void)time(&the_time);
	tm_ptr = localtime(&the_time);
	result = strftime(buf, sizeof(buf), "%A %d %B, %I:%S %p", tm_ptr);

	if (result != 0) {
		printf("strftime gives: %s\n", buf);
		puts("PASS");
		return PTS_PASS;
	} else {
		puts("FAIL");
		return PTS_FAIL;
	}
}
