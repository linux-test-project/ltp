// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

#include <time.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_clocks.h"
#include "lapi/syscalls.h"

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
