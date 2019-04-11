// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Linaro Limited. All rights reserved.
 * Author: Rafael David Tinoco <rafael.tinoco@linaro.org>
 */

#include "config.h"
#include "tst_test.h"
#include "tst_timer.h"
#include "tst_safe_clocks.h"
#include "lapi/syscalls.h"
#include "lapi/posix_clocks.h"
#include <time.h>
#include <pwd.h>
#include <sys/timex.h>
#include <sys/types.h>
#include "lapi/timex.h"

static int sys_clock_adjtime(clockid_t, struct timex *);
static void timex_show(char *, struct timex);

/*
 * bad pointer w/ libc causes SIGSEGV signal, call syscall directly
 */
static int sys_clock_adjtime(clockid_t clk_id, struct timex *txc)
{
	return tst_syscall(__NR_clock_adjtime, clk_id, txc);
}

static void timex_show(char *given, struct timex txc)
{
	tst_res(TINFO,  "%s\n"
			"             mode: %d\n"
			"           offset: %ld\n"
			"        frequency: %ld\n"
			"         maxerror: %ld\n"
			"         esterror: %ld\n"
			"           status: %d (0x%x)\n"
			"    time_constant: %ld\n"
			"        precision: %ld\n"
			"        tolerance: %ld\n"
			"             tick: %ld\n"
			"         raw time: %d(s) %d(us)",
			given,
			txc.modes,
			txc.offset,
			txc.freq,
			txc.maxerror,
			txc.esterror,
			txc.status,
			txc.status,
			txc.constant,
			txc.precision,
			txc.tolerance,
			txc.tick,
			(int)txc.time.tv_sec,
			(int)txc.time.tv_usec);
}
