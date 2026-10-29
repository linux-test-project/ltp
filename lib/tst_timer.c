// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 */

#include <errno.h>

#define TST_NO_DEFAULT_MAIN

#include "tst_test.h"
#include "tst_timer.h"
#include "tst_clocks.h"
#include "lapi/posix_clocks.h"

static struct timespec start_time, stop_time;
static clockid_t clock_id;

void tst_timer_check(clockid_t clk_id)
{
	if (tst_clock_gettime(clk_id, &start_time)) {
		if (errno == EINVAL) {
			tst_brk(TCONF,
			         "Clock id %s(%u) not supported by kernel",
				 tst_clock_name(clk_id), clk_id);
			return;
		}

		tst_brk(TBROK | TERRNO, "tst_clock_gettime() failed");
	}
}

void tst_timer_start(clockid_t clk_id)
{
	clock_id = clk_id;

	if (tst_clock_gettime(clock_id, &start_time))
		tst_res(TWARN | TERRNO, "tst_clock_gettime() failed");
}

int tst_timer_expired_ms(long long ms)
{
	struct timespec cur_time;

	if (tst_clock_gettime(clock_id, &cur_time))
		tst_res(TWARN | TERRNO, "tst_clock_gettime() failed");

	return tst_timespec_diff_ms(cur_time, start_time) >= ms;
}

void tst_timer_stop(void)
{
	if (tst_clock_gettime(clock_id, &stop_time))
		tst_res(TWARN | TERRNO, "tst_clock_gettime() failed");
}

struct timespec tst_timer_elapsed(void)
{
	return tst_timespec_diff(stop_time, start_time);
}
