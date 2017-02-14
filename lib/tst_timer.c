/*
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <errno.h>

#include "test.h"
#include "tst_timer.h"
#include "lapi/posix_clocks.h"

static struct timespec start_time, stop_time;
static clockid_t clock_id;

static const char *clock_name(clockid_t clk_id)
{
	switch (clk_id) {
	case CLOCK_REALTIME:
		return "CLOCK_REALTIME";
	case CLOCK_REALTIME_COARSE:
		return "CLOCK_REALTIME_COARSE";
	case CLOCK_MONOTONIC:
		return "CLOCK_MONOTONIC";
	case CLOCK_MONOTONIC_COARSE:
		return "CLOCK_MONOTONIC_COARSE";
	case CLOCK_MONOTONIC_RAW:
		return "CLOCK_MONOTONIC_RAW";
	case CLOCK_BOOTTIME:
		return "CLOCK_BOOTTIME";
	case CLOCK_PROCESS_CPUTIME_ID:
		return "CLOCK_PROCESS_CPUTIME_ID";
	case CLOCK_THREAD_CPUTIME_ID:
		return "CLOCK_THREAD_CPUTIME_ID";
	default:
		return "UNKNOWN/INVALID";
	}
}

void tst_timer_check(clockid_t clk_id)
{
	if (clock_gettime(clk_id, &start_time)) {
		if (errno == EINVAL) {
			tst_brkm(TCONF, NULL,
			         "Clock id %s(%u) not supported by kernel",
				 clock_name(clk_id), clk_id);
			return;
		}

		tst_brkm(TBROK | TERRNO, NULL, "clock_gettime() failed");
	}
}

void tst_timer_start(clockid_t clk_id)
{
	clock_id = clk_id;

	if (clock_gettime(clock_id, &start_time))
		tst_resm(TWARN | TERRNO, "clock_gettime() failed");
}

void tst_timer_stop(void)
{
	if (clock_gettime(clock_id, &stop_time))
		tst_resm(TWARN | TERRNO, "clock_gettime() failed");
}

struct timespec tst_timer_elapsed(void)
{
	return tst_timespec_diff(stop_time, start_time);
}
