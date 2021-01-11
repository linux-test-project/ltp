// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2020 Unisoc Communications Inc.
 *
 * This file is a implementation for rtc set read,covert to tm functions
 */

#include <stdbool.h>
#include <limits.h>
#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_rtctime.h"

#define LEAPS_THRU_END_OF(y) ((y) / 4 - (y) / 100 + (y) / 400)

static const unsigned char rtc_days_in_month[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static inline bool is_leap_year(unsigned int year)
{
	return (!(year % 4) && (year % 100)) || !(year % 400);
}

static long long tst_mktime(const unsigned int year0, const unsigned int mon0,
		const unsigned int day, const unsigned int hour,
		const unsigned int min, const unsigned int sec)
{
	unsigned int mon = mon0, year = year0;

	/* 1..12 -> 11,12,1..10 */
	mon -= 2;
	if (0 >= (int) (mon)) {
		mon += 12;	/* Puts Feb last since it has leap day */
		year -= 1;
	}

	return ((((long long)
		(year/4 - year/100 + year/400 + 367*mon/12 + day) +
		year*365 - 719499
		)*24 + hour /* now have hours - midnight tomorrow handled here */
		)*60 + min /* now have minutes */
		)*60 + sec; /* finally seconds */
}

/*
 * The number of days in the month.
 */
static int rtc_month_days(unsigned int month, unsigned int year)
{
	return rtc_days_in_month[month] + (is_leap_year(year) && month == 1);
}

/*
 * tst_rtc_time_to_tm - Converts time_t to rtc_time.
 * Convert seconds since 01-01-1970 00:00:00 to Gregorian date.
 */
void tst_rtc_time_to_tm(long long time, struct rtc_time *tm)
{
	unsigned int month, year, secs;
	int days;

	/* time must be positive */
	days = time / 86400;
	secs = time % 86400;

	/* day of the week, 1970-01-01 was a Thursday */
	tm->tm_wday = (days + 4) % 7;

	year = 1970 + days / 365;
	days -= (year - 1970) * 365
		+ LEAPS_THRU_END_OF(year - 1)
		- LEAPS_THRU_END_OF(1970 - 1);
	while (days < 0) {
		year -= 1;
		days += 365 + is_leap_year(year);
	}
	tm->tm_year = year - 1900;
	tm->tm_yday = days + 1;

	for (month = 0; month < 11; month++) {
		int newdays;

		newdays = days - rtc_month_days(month, year);
		if (newdays < 0)
			break;
		days = newdays;
	}
	tm->tm_mon = month;
	tm->tm_mday = days + 1;

	tm->tm_hour = secs / 3600;
	secs -= tm->tm_hour * 3600;
	tm->tm_min = secs / 60;
	tm->tm_sec = secs - tm->tm_min * 60;

	tm->tm_isdst = 0;
}

/*
 * tst_rtc_tm_to_time - Converts rtc_time to time_t.
 * Convert Gregorian date to seconds since 01-01-1970 00:00:00.
 */
long long tst_rtc_tm_to_time(struct rtc_time *tm)
{
	return tst_mktime(((unsigned int)tm->tm_year + 1900), tm->tm_mon + 1,
			tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
}

int tst_rtc_ioctl(const char *rtc_dev, unsigned long request,
                  struct rtc_time *rtc_tm)
{
	int ret;
	int rtc_fd = -1;

	rtc_fd = SAFE_OPEN(rtc_dev, O_RDONLY);

	ret = ioctl(rtc_fd, request, rtc_tm);

	if (ret != 0)
		return -1;

	if (rtc_fd > 0)
		SAFE_CLOSE(rtc_fd);

	return 0;
}
