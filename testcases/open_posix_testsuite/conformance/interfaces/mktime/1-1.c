/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 *
 * This test case shall return PASS on converting the broken down July 4th 2001
 * into a time since the Epoch, which is the same encoding as of the value
 * returned by time(), otherwise it fails with -1.
 */

#include <stdio.h>
#include <time.h>
#include "posixtest.h"

struct tm tm_ptr;
time_t tps;

int main(void)
{
	/* Break down July 4th, 2001 */
	tm_ptr.tm_year = 2001 - 1900;
	tm_ptr.tm_mon = 7 - 1;
	tm_ptr.tm_mday = 4;
	tm_ptr.tm_hour = 0;
	tm_ptr.tm_min = 0;
	tm_ptr.tm_sec = 1;
	tm_ptr.tm_isdst = -1;

	tps = mktime(&tm_ptr);

	if (tps != -1) {
		printf("%s", ctime(&tps));
		puts("TEST PASSED");
		return PTS_PASS;
	} else {
		puts("TEST FAILED");
		return PTS_FAIL;
	}
}
