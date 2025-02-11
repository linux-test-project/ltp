// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2020 Unisoc Communications Inc.
 */

/*\
 * RTC device set time function test.
 *
 * [Algorithm]
 *
 * - Save RTC time
 * - Set RTC time
 * - Read the RTC time back
 * - Check if the set time and the read time are identical
 * - Restore RTC time
 */

#include <stdio.h>
#include "tst_rtctime.h"
#include "tst_wallclock.h"
#include "tst_test.h"

static char *rtc_dev = "/dev/rtc";

static char *rtctime_to_str(struct rtc_time *tm)
{
	static char rtctime_buf[128];

	sprintf(rtctime_buf, "%04d-%02d-%02d %02d:%02d:%02d",
		tm->tm_year + 1900,
		tm->tm_mon + 1,
		tm->tm_mday,
		tm->tm_hour,
		tm->tm_min,
		tm->tm_sec);
	return rtctime_buf;
}

static int rtc_tm_cmp(struct rtc_time *set_tm, struct rtc_time *read_tm)
{
	long delta, seconds1, seconds2;

	if (set_tm->tm_year != read_tm->tm_year)
		return 1;

	if (set_tm->tm_mon != read_tm->tm_mon)
		return 1;

	if (set_tm->tm_mday != read_tm->tm_mday)
		return 1;

	/*
	 * Convert hour/min/sec into seconds to handle the normal
	 * and special situations:
	 * 1#
	 *       set_tm:  2022-04-28 13:00:50
	 *       read_tm: 2022-04-28 13:00:50
	 * 2#
	 *       set_tm:  2022-04-28 13:00:50
	 *       read_tm: 2022-04-28 13:00:51
	 * 3#
	 *       set_tm:  2022-04-28 13:00:59
	 *       read_tm: 2022-04-28 13:01:00
	 * 4#
	 *       set_tm:  2022-04-28 13:59:59
	 *       read_tm: 2022-04-28 14:00:00
	 *
	 * Note: as we have avoided testing around the zero
	 * clock, so it's impossible to hit situation 5#
	 *       set_tm:  2022-04-28 23:59:59
	 *       read_tm: 2022-04-29 00:00:00
	 */
	if ((set_tm->tm_hour != read_tm->tm_hour)
		|| (set_tm->tm_min != read_tm->tm_min)
		|| (set_tm->tm_sec != read_tm->tm_sec)) {

		seconds1 = (set_tm->tm_hour  * 3600) + (set_tm->tm_min  * 60) + set_tm->tm_sec;
		seconds2 = (read_tm->tm_hour * 3600) + (read_tm->tm_min * 60) + read_tm->tm_sec;

		delta = seconds2 - seconds1;

		if (delta < 0 || delta > 3) {
			tst_res(TFAIL, "seconds1 is %ld, seconds2 is %ld", seconds1, seconds2);
			return 1;
		}
	}

	return 0;
}

static void set_rtc_test(void)
{
	struct rtc_time read_tm, set_tm;
	int ret;

	/* Read current RTC Time */
	ret = tst_rtc_gettime(rtc_dev, &read_tm);
	if (ret != 0) {
		tst_res(TFAIL | TERRNO, "ioctl() RTC_RD_TIME");
		return;
	}

	/* set rtc to +/-1 hour */
	set_tm = read_tm;
	if (set_tm.tm_hour == 0)
		set_tm.tm_hour += 1;
	else
		set_tm.tm_hour -= 1;

	tst_res(TINFO, "To set RTC date/time is: %s", rtctime_to_str(&set_tm));

	ret = tst_rtc_settime(rtc_dev, &set_tm);
	if (ret != 0) {
		tst_res(TFAIL | TERRNO, "ioctl() RTC_SET_TIME");
		return;
	}

	/* Read new RTC Time */
	ret = tst_rtc_gettime(rtc_dev, &read_tm);
	if (ret != 0) {
		tst_res(TFAIL | TERRNO, "ioctl() RTC_RD_TIME");
		return;
	}
	tst_res(TINFO, "read RTC date/time is: %s", rtctime_to_str(&read_tm));

	if (rtc_tm_cmp(&set_tm, &read_tm)) {
		tst_res(TFAIL, "RTC SET TEST");
		return;
	}
	tst_res(TPASS, "The read RTC time is consistent with set time");
}

static void rtc_setup(void)
{
	int exists = access(rtc_dev, F_OK);

	if (exists < 0)
		tst_brk(TCONF, "RTC device %s not available", rtc_dev);

	tst_rtc_clock_save(rtc_dev);
}

static void rtc_cleanup(void)
{
	tst_rtc_clock_restore(rtc_dev);
}

static struct tst_test test = {
	.setup = rtc_setup,
	.test_all = set_rtc_test,
	.cleanup = rtc_cleanup,
	.needs_root = 1,
};
