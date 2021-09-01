// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Linaro Limited. All rights reserved.
 * Author: Rafael David Tinoco <rafael.tinoco@linaro.org>
 */

#include <errno.h>

#define TST_NO_DEFAULT_MAIN

#include "tst_test.h"
#include "tst_timer.h"
#include "tst_clocks.h"
#include "tst_rtctime.h"
#include "tst_wallclock.h"
#include "lapi/posix_clocks.h"

static struct timespec real_begin, mono_begin;

static struct rtc_time rtc_begin;

static int clock_saved;

void tst_wallclock_save(void)
{
	/* save initial monotonic time to restore it when needed */

	if (tst_clock_gettime(CLOCK_REALTIME, &real_begin))
		tst_brk(TBROK | TERRNO, "tst_clock_gettime() realtime failed");

	if (tst_clock_gettime(CLOCK_MONOTONIC_RAW, &mono_begin)) {
		if (errno == EINVAL) {
			tst_brk(TCONF | TERRNO,
				"tst_clock_gettime() didn't support CLOCK_MONOTONIC_RAW");
		}

		tst_brk(TBROK | TERRNO, "tst_clock_gettime() monotonic failed");
	}

	clock_saved = 1;
}

void tst_wallclock_restore(void)
{
	static const char *localtime = "/etc/localtime";
	static struct timespec mono_end, elapsed, adjust;
	int ret;

	if (!clock_saved)
		return;

	clock_saved = 0;

	if (tst_clock_gettime(CLOCK_MONOTONIC_RAW, &mono_end))
		tst_brk(TBROK | TERRNO, "tst_clock_gettime() monotonic failed");

	elapsed = tst_timespec_diff(mono_end, mono_begin);

	adjust = tst_timespec_add(real_begin, elapsed);

	/* restore realtime clock based on monotonic delta */

	if (tst_clock_settime(CLOCK_REALTIME, &adjust))
		tst_brk(TBROK | TERRNO, "tst_clock_settime() realtime failed");

	/*
	 * Fix access time of /etc/localtime because adjusting the wallclock
	 * might have changed it to a time value which lies far ahead
	 * in the future.
	 * The access time of a file only changes if the new one is past
	 * the current one, therefore, just opening a file and reading it
	 * might not be enough because the current access time might be far
	 * in the future.
	 */
	ret = access(localtime, F_OK | W_OK);
	if (!ret)
		SAFE_TOUCH(localtime, 0, NULL);
}

void tst_rtc_clock_save(const char *rtc_dev)
{
	/* save initial monotonic time to restore it when needed */
	if (tst_rtc_gettime(rtc_dev, &rtc_begin))
		tst_brk(TBROK | TERRNO, "tst_rtc_gettime() realtime failed");

	if (tst_clock_gettime(CLOCK_MONOTONIC_RAW, &mono_begin))
		tst_brk(TBROK | TERRNO, "tst_clock_gettime() monotonic failed");

	clock_saved = 1;
}

void tst_rtc_clock_restore(const char *rtc_dev)
{
	static struct timespec mono_end, elapsed;
	static struct timespec rtc_begin_tm, rtc_adjust;
	static struct rtc_time rtc_restore;

	if (!clock_saved)
		return;

	clock_saved = 0;

	if (tst_clock_gettime(CLOCK_MONOTONIC_RAW, &mono_end))
		tst_brk(TBROK | TERRNO, "tst_clock_gettime() monotonic failed");

	elapsed = tst_timespec_diff(mono_end, mono_begin);

	rtc_begin_tm.tv_nsec = 0;
	rtc_begin_tm.tv_sec = tst_rtc_tm_to_time(&rtc_begin);

	rtc_adjust = tst_timespec_add(rtc_begin_tm, elapsed);

	tst_rtc_time_to_tm(rtc_adjust.tv_sec, &rtc_restore);

	/* restore realtime clock based on monotonic delta */
	if (tst_rtc_settime(rtc_dev, &rtc_restore))
		tst_brk(TBROK | TERRNO, "tst_rtc_settime() realtime failed");
}
