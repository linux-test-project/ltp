// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Linaro Limited. All rights reserved.
 * Author: Rafael David Tinoco <rafael.tinoco@linaro.org>
 */

/*
 * Basic test for clock_settime(2) on REALTIME clock:
 *
 *      1) advance DELTA_SEC seconds
 *      2) go backwards DELTA_SEC seconds
 *
 * Restore wall clock at the end of test.
 */

#include "config.h"
#include "tst_timer.h"
#include "tst_safe_clocks.h"
#include "tst_test.h"
#include "lapi/syscalls.h"

#define DELTA_SEC 10
#define DELTA_US (long long) (DELTA_SEC * 1000000)
#define DELTA_EPS (long long) (DELTA_US * 0.1)

static struct timespec *begin, *change, *end;

static void verify_clock_settime(void)
{
	long long elapsed;

	/* test 01: move forward */

	SAFE_CLOCK_GETTIME(CLOCK_REALTIME, begin);

	*change = tst_timespec_add_us(*begin, DELTA_US);

	if (clock_settime(CLOCK_REALTIME, change) != 0)
		tst_brk(TBROK | TTERRNO, "could not set realtime change");

	SAFE_CLOCK_GETTIME(CLOCK_REALTIME, end);

	elapsed = tst_timespec_diff_us(*end, *begin);

	if (elapsed >= DELTA_US && elapsed < (DELTA_US + DELTA_EPS))
		tst_res(TPASS, "clock_settime(2): was able to advance time");
	else
		tst_res(TFAIL, "clock_settime(2): could not advance time");

	/* test 02: move backward */

	SAFE_CLOCK_GETTIME(CLOCK_REALTIME, begin);

	*change = tst_timespec_sub_us(*begin, DELTA_US);

	if (clock_settime(CLOCK_REALTIME, change) != 0)
		tst_brk(TBROK | TTERRNO, "could not set realtime change");

	SAFE_CLOCK_GETTIME(CLOCK_REALTIME, end);

	elapsed = tst_timespec_diff_us(*end, *begin);

	if (~(elapsed) <= DELTA_US && ~(elapsed) > (DELTA_US - DELTA_EPS))
		tst_res(TPASS, "clock_settime(2): was able to recede time");
	else
		tst_res(TFAIL, "clock_settime(2): could not recede time");
}

static struct tst_test test = {
	.test_all = verify_clock_settime,
	.needs_root = 1,
	.restore_wallclock = 1,
	.bufs = (struct tst_buffers []) {
		{&begin, .size = sizeof(*begin)},
		{&change, .size = sizeof(*change)},
		{&end, .size = sizeof(*end)},
		{},
	}
};
