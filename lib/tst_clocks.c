// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

#include <time.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_clocks.h"
#include "lapi/syscalls.h"
#include "lapi/posix_clocks.h"

int tst_clock_getres(clockid_t clk_id, struct timespec *res)
{
	return tst_syscall(__NR_clock_getres, clk_id, res);
}

int tst_clock_gettime(clockid_t clk_id, struct timespec *ts)
{
	return tst_syscall(__NR_clock_gettime, clk_id, ts);
}

int tst_clock_settime(clockid_t clk_id, struct timespec *ts)
{
	return tst_syscall(__NR_clock_settime, clk_id, ts);
}

const char *tst_clock_name(clockid_t clk_id)
{
	switch (clk_id) {
	case CLOCK_REALTIME:
		return "CLOCK_REALTIME";
	case CLOCK_MONOTONIC:
		return "CLOCK_MONOTONIC";
	case CLOCK_PROCESS_CPUTIME_ID:
		return "CLOCK_PROCESS_CPUTIME_ID";
	case CLOCK_THREAD_CPUTIME_ID:
		return "CLOCK_THREAD_CPUTIME_ID";
	case CLOCK_MONOTONIC_RAW:
		return "CLOCK_MONOTONIC_RAW";
	case CLOCK_REALTIME_COARSE:
		return "CLOCK_REALTIME_COARSE";
	case CLOCK_MONOTONIC_COARSE:
		return "CLOCK_MONOTONIC_COARSE";
	case CLOCK_BOOTTIME:
		return "CLOCK_BOOTTIME";
	case CLOCK_REALTIME_ALARM:
		return "CLOCK_REALTIME_ALARM";
	case CLOCK_BOOTTIME_ALARM:
		return "CLOCK_BOOTTIME_ALARM";
	case CLOCK_TAI:
		return "CLOCK_TAI";
	default:
		return "INVALID/UNKNOWN CLOCK";
	}
}
