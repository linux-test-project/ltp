/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 *
 * The names in the broken-down tm structure should correspond to its values.
 *
 */

#include <stdio.h>
#include <time.h>
#include <posixtest.h>

int main(void)
{
	struct tm *tm_ptr;
	time_t the_time;
	int total_years;

	(void)time(&the_time);
	tm_ptr = gmtime(&the_time);
	printf("Raw time is %ld \n", the_time);
	printf("gmtime gives:\n");

	/* Checking the seconds */
	if ((tm_ptr->tm_sec >= 0) && (tm_ptr->tm_sec < 60)) {
		printf("sec %02d\n", tm_ptr->tm_sec);
	} else {
		puts("Test FAILED: seconds");
		return PTS_FAIL;
	}

	/* Checking the Minutes */
	if ((tm_ptr->tm_min >= 0) && (tm_ptr->tm_min <= 59)) {
		printf("min %02d\n", tm_ptr->tm_min);
	} else {
		puts("Test FAILED: minutes");
		return PTS_FAIL;
	}

	/* Checking the Hour */
	if ((tm_ptr->tm_hour >= 0) && (tm_ptr->tm_hour <= 23)) {
		printf("hour %02d\n", tm_ptr->tm_hour);
	} else {
		puts("Test FAILED: hour");
		return PTS_FAIL;
	}

	/* Checking the Month Day */
	if ((tm_ptr->tm_mday >= 1) && (tm_ptr->tm_mday <= 31)) {
		printf("mday %02d\n", tm_ptr->tm_mday);
	} else {
		puts("Test FAILED: mday");
		return PTS_FAIL;
	}

	/* Checking the Month */
	if ((tm_ptr->tm_mon >= 0) && (tm_ptr->tm_mon <= 11)) {
		printf("mon %02d\n", tm_ptr->tm_mon);
	} else {
		puts("Test FAILED: mon");
		return PTS_FAIL;
	}

	/* Checking the Year */
	total_years = (tm_ptr->tm_year + 1900);
	if (total_years >= 1900) {
		printf("year %d\n", total_years);
	} else {
		printf("year %d\n", total_years);
		puts("Test FAILED: year");
		return PTS_FAIL;
	}

	/* Checking the Day of week */
	if ((tm_ptr->tm_wday >= 0) && (tm_ptr->tm_wday <= 6)) {
		printf("wday %02d\n", tm_ptr->tm_wday);
	} else {
		puts("Test FAILED: wday");
		return PTS_FAIL;
	}

	/* Checking the Day in year */
	if ((tm_ptr->tm_yday >= 0) && (tm_ptr->tm_yday <= 365)) {
		printf("yday %02d\n", tm_ptr->tm_yday);
	} else {
		puts("Test FAILED: yday");
		return PTS_FAIL;
	}

	/* Checking the DTS */
	if ((tm_ptr->tm_isdst >= -1) && (tm_ptr->tm_isdst <= 1)) {
		printf("isdst %02d\n", tm_ptr->tm_isdst);
	} else {
		puts("Test FAILED: isdst");
		return PTS_FAIL;
	}

	puts("Test PASSED");
	return PTS_PASS;
}
