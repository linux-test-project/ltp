// SPDX-License-Identifier: GPL-2.0 or later
/*
 *  Copyright (c) Zilogic Systems Pvt. Ltd., 2018
 *  Email : code@zilogic.com
 */

#include <time.h>
#include "tst_test.h"

static inline void safe_clock_getres(const char *file, const int lineno,
	clockid_t clk_id, struct timespec *res)
{
	int rval;

	rval = clock_getres(clk_id, res);
	if (rval != 0)
		tst_brk(TBROK | TERRNO,
			"%s:%d clock_getres() failed", file, lineno);

}

static inline void safe_clock_gettime(const char *file, const int lineno,
	clockid_t clk_id, struct timespec *tp)
{
	int rval;

	rval = clock_gettime(clk_id, tp);
	if (rval != 0)
		tst_brk(TBROK | TERRNO,
			"%s:%d clock_gettime() failed", file, lineno);
}

#define SAFE_CLOCK_GETRES(clk_id, res)\
	safe_clock_getres(__FILE__, __LINE__, (clk_id), (res))

#define SAFE_CLOCK_GETTIME(clk_id, tp)\
	safe_clock_gettime(__FILE__, __LINE__, (clk_id), (tp))

