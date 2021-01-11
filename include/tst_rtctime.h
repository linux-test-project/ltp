/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2020 Unisoc Inc.
 */

#ifndef TST_RTCTIME
#define TST_RTCTIME

#include <sys/ioctl.h>
#include <linux/rtc.h>

int tst_rtc_ioctl(const char *rtc_dev, unsigned long request,
                  struct rtc_time *rtc_tm);

static inline int tst_rtc_gettime(const char *rtc_dev, struct rtc_time *rtc_tm)
{
	return tst_rtc_ioctl(rtc_dev, RTC_RD_TIME, rtc_tm);
}

static inline int tst_rtc_settime(const char *rtc_dev, struct rtc_time *rtc_tm)
{
	return tst_rtc_ioctl(rtc_dev, RTC_SET_TIME, rtc_tm);
}

void tst_rtc_time_to_tm(long long time, struct rtc_time *tm);

long long tst_rtc_tm_to_time(struct rtc_time *tm);

#endif /* TST_RTCTIME */
