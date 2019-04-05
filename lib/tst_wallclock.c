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
#include "tst_wallclock.h"
#include "lapi/posix_clocks.h"

static struct timespec real_begin, mono_begin;

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
	static struct timespec mono_end, elapsed, adjust;

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
}
