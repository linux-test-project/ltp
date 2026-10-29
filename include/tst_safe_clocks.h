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

static inline int safe_clock_getres(const char *file, const int lineno,
	clockid_t clk_id, struct timespec *res)
{
	int rval;

	rval = clock_getres(clk_id, res);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"clock_getres(%s) failed", tst_clock_name(clk_id));
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid clock_getres(%s) return value %d",
			tst_clock_name(clk_id), rval);
	}

	return rval;
}

static inline int safe_clock_gettime(const char *file, const int lineno,
	clockid_t clk_id, struct timespec *tp)
{
	int rval;

	rval = clock_gettime(clk_id, tp);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"clock_gettime(%s) failed", tst_clock_name(clk_id));
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid clock_gettime(%s) return value %d",
			tst_clock_name(clk_id), rval);
	}

	return rval;
}


static inline int safe_clock_settime(const char *file, const int lineno,
	clockid_t clk_id, struct timespec *tp)
{
	int rval;

	rval = clock_settime(clk_id, tp);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"clock_gettime(%s) failed", tst_clock_name(clk_id));
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid clock_gettime(%s) return value %d",
			tst_clock_name(clk_id), rval);
	}

	return rval;
}

static inline int safe_clock_nanosleep(const char *file, const int lineno,
	clockid_t clockid, int flags, const struct timespec *ts,
	struct timespec *remain)
{
	int ret;

	errno = 0;
	ret = clock_nanosleep(clockid, flags, ts, remain);

	if (ret == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"clock_nanosleep() failed");
	} else if (ret) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid clock_nanosleep() return value %d", ret);
	}

	return ret;
}

static inline int safe_timer_create(const char *file, const int lineno,
	clockid_t clockid, struct sigevent *sevp, timer_t *timerid)
{
	int ret;

	errno = 0;
	ret = timer_create(clockid, sevp, timerid);

	if (ret == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"timer_create(%s) failed", tst_clock_name(clockid));
	} else if (ret) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid timer_create(%s) return value %d",
			tst_clock_name(clockid), ret);
	}

	return ret;
}

static inline int safe_timer_settime(const char *file, const int lineno,
	timer_t timerid, int flags, const struct itimerspec *new_value,
	struct itimerspec *old_value)
{
	int ret;

	errno = 0;
	ret = timer_settime(timerid, flags, new_value, old_value);

	if (ret == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"timer_settime() failed");
	} else if (ret) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid timer_settime() return value %d", ret);
	}

	return ret;
}

static inline int safe_timer_gettime(const char *file, const int lineno,
	timer_t timerid, struct itimerspec *curr_value)
{
	int ret;

	errno = 0;
	ret = timer_gettime(timerid, curr_value);

	if (ret == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"timer_gettime() failed");
	} else if (ret) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid timer_gettime() return value %d", ret);
	}

	return ret;
}

static inline int safe_timer_delete(const char *file, const int lineno,
	timer_t timerid)
{
	int ret;

	errno = 0;
	ret = timer_delete(timerid);

	if (ret == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO, "timer_delete() failed");
	} else if (ret) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid timer_delete() return value %d", ret);
	}

	return ret;
}

#define SAFE_CLOCK_GETRES(clk_id, res)\
	safe_clock_getres(__FILE__, __LINE__, (clk_id), (res))

#define SAFE_CLOCK_GETTIME(clk_id, tp)\
	safe_clock_gettime(__FILE__, __LINE__, (clk_id), (tp))

#define SAFE_CLOCK_SETTIME(clk_id, tp)\
	safe_clock_settime(__FILE__, __LINE__, (clk_id), (tp))

#define SAFE_CLOCK_NANOSLEEP(clockid, flags, ts, remain)\
	safe_clock_nanosleep(__FILE__, __LINE__, clockid, flags, ts, remain)

#define SAFE_TIMER_CREATE(clockid, sevp, timerid)\
	safe_timer_create(__FILE__, __LINE__, (clockid), (sevp), (timerid))

#define SAFE_TIMER_SETTIME(timerid, flags, new_value, old_value)\
	safe_timer_settime(__FILE__, __LINE__, (timerid), (flags),\
		(new_value), (old_value))

#define SAFE_TIMER_GETTIME(timerid, curr_value)\
	safe_timer_gettime(__FILE__, __LINE__, (timerid), (curr_value))

#define SAFE_TIMER_DELETE(timerid)\
	safe_timer_delete(__FILE__, __LINE__, timerid)

#endif /* SAFE_CLOCKS_H__ */
