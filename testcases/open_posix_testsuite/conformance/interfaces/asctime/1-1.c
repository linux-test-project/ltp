/*
 * Copyright (c) 2002-3, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

The asctime() function shall convert the broken-down time in the structure pointed to by timeptr into a string in the form: Sun Sep 16 01:03:52 1973\n\0

 */

#define WEEKDAY 0
#define MONTH 8
#define MONTHDAY 16
#define HOUR 1
#define MINUTE 3
#define SECOND 52
#define YEAR 73

#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

int main(void)
{
	struct tm time_ptr;

	char expected[128];
	char *real;

	char wday_name[7][3] =
	    { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

	char mon_name[12][3] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};

	time_ptr.tm_wday = WEEKDAY;
	time_ptr.tm_mon = MONTH;
	time_ptr.tm_mday = MONTHDAY;
	time_ptr.tm_hour = HOUR;
	time_ptr.tm_min = MINUTE;
	time_ptr.tm_sec = SECOND;
	time_ptr.tm_year = YEAR;

	real = asctime(&time_ptr);

	sprintf(expected, "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n",
		wday_name[time_ptr.tm_wday],
		mon_name[time_ptr.tm_mon],
		time_ptr.tm_mday, time_ptr.tm_hour,
		time_ptr.tm_min, time_ptr.tm_sec, 1900 + time_ptr.tm_year);

	printf("real = %s\n", real);
	printf("expected = %s\n", expected);

	if (strcmp(real, expected) != 0) {
		perror("asctime did not return the correct value\n");
		printf("Got %s\n", real);
		printf("Expected %s\n", expected);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
