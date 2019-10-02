// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * clock_gettime() and clock_getres() functions
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <time.h>
#include <sys/syscall.h>

#include "tst_clocks.h"

int tst_clock_getres(clockid_t clk_id, struct timespec *res)
{
	return syscall(SYS_clock_getres, clk_id, res);
}

int tst_clock_gettime(clockid_t clk_id, struct timespec *ts)
{
	return syscall(SYS_clock_gettime, clk_id, ts);
}

int tst_clock_settime(clockid_t clk_id, struct timespec *ts)
{
	return syscall(SYS_clock_settime, clk_id, ts);
}
