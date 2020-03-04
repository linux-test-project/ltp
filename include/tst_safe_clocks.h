// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019, Linux Test Project
 * Copyright (c) Zilogic Systems Pvt. Ltd., 2018
 * Email : code@zilogic.com
 */

#ifndef TST_SAFE_CLOCKS_H__
#define TST_SAFE_CLOCKS_H__

#include <time.h>
#include <sys/timex.h>
#include "tst_test.h"
#include "tst_clocks.h"
#include "lapi/syscalls.h"
#include "lapi/posix_clocks.h"

static inline void safe_clock_getres(const char *file, const int lineno,
	clockid_t clk_id, struct timespec *res)
{
	int rval;

	rval = clock_getres(clk_id, res);
	if (rval != 0) {
		tst_brk(TBROK | TERRNO,
			"%s:%d clock_getres(%s) failed",
			file, lineno, tst_clock_name(clk_id));
	}
}

static inline void safe_clock_gettime(const char *file, const int lineno,
	clockid_t clk_id, struct timespec *tp)
{
	int rval;

	rval = clock_gettime(clk_id, tp);
	if (rval != 0) {
		tst_brk(TBROK | TERRNO,
			"%s:%d clock_gettime(%s) failed",
			file, lineno, tst_clock_name(clk_id));
	}
}


static inline void safe_clock_settime(const char *file, const int lineno,
	clockid_t clk_id, struct timespec *tp)
{
	int rval;

	rval = clock_settime(clk_id, tp);
	if (rval != 0) {
		tst_brk(TBROK | TERRNO,
			"%s:%d clock_gettime(%s) failed",
			file, lineno, tst_clock_name(clk_id));
	}
}

static inline int safe_clock_adjtime(const char *file, const int lineno,
	clockid_t clk_id, struct timex *txc)
{
	int rval;

	rval = tst_syscall(__NR_clock_adjtime, clk_id, txc);
	if (rval < 0) {
		tst_brk(TBROK | TERRNO,
			"%s:%d clock_adjtime(%s) failed %i",
			file, lineno, tst_clock_name(clk_id), rval);
	}

	return rval;
}

#define SAFE_CLOCK_GETRES(clk_id, res)\
	safe_clock_getres(__FILE__, __LINE__, (clk_id), (res))

#define SAFE_CLOCK_GETTIME(clk_id, tp)\
	safe_clock_gettime(__FILE__, __LINE__, (clk_id), (tp))

#define SAFE_CLOCK_SETTIME(clk_id, tp)\
	safe_clock_settime(__FILE__, __LINE__, (clk_id), (tp))

#define SAFE_CLOCK_ADJTIME(clk_id, txc)\
	safe_clock_adjtime(__FILE__, __LINE__, (clk_id), (txc))

#endif /* SAFE_CLOCKS_H__ */
