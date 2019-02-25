// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Linaro Limited. All rights reserved.
 * Author: Rafael David Tinoco <rafael.tinoco@linaro.org>
 */
/*
 * Basic test for clock_gettime(2) on multiple clocks:
 *
 *  1) CLOCK_REALTIME
 *  2) CLOCK_MONOTONIC
 *  3) CLOCK_PROCESS_CPUTIME_ID
 *  4) CLOCK_THREAD_CPUTIME_ID
 *  5) CLOCK_REALTIME_COARSE
 *  6) CLOCK_MONOTONIC_COARSE
 *  7) CLOCK_MONOTONIC_RAW
 *  8) CLOCK_BOOTTIME
 */

#include "config.h"
#include "tst_timer.h"
#include "tst_safe_clocks.h"
#include "tst_test.h"
#include "lapi/syscalls.h"

struct test_case {
	clockid_t clktype;
	int allow_inval;
};

struct tmpfunc {
	int (*func)(clockid_t clk_id, struct timespec *tp);
	char *desc;
};

struct test_case tc[] = {
	{
	 .clktype = CLOCK_REALTIME,
	 },
	{
	 .clktype = CLOCK_MONOTONIC,
	 },
	{
	 .clktype = CLOCK_PROCESS_CPUTIME_ID,
	 },
	{
	 .clktype = CLOCK_THREAD_CPUTIME_ID,
	 },
	{
	 .clktype = CLOCK_REALTIME_COARSE,
	 .allow_inval = 1,
	 },
	{
	 .clktype = CLOCK_MONOTONIC_COARSE,
	 .allow_inval = 1,
	 },
	{
	 .clktype = CLOCK_MONOTONIC_RAW,
	 .allow_inval = 1,
	 },
	{
	 .clktype = CLOCK_BOOTTIME,
	 .allow_inval = 1,
	 },
};

static int sys_clock_gettime(clockid_t clk_id, struct timespec *tp)
{
	return tst_syscall(__NR_clock_gettime, clk_id, tp);
}

static int check_spec(struct timespec *spec)
{
	return (spec->tv_nsec != 0 || spec->tv_sec != 0) ? 1 : 0;
}

static void verify_clock_gettime(unsigned int i)
{
	size_t sz;
	struct timespec spec;

	/*
	 * check clock_gettime() syscall AND libc (or vDSO) functions
	 */
	struct tmpfunc tf[] = {
		{ .func = sys_clock_gettime, .desc = "syscall"      },
		{ .func = clock_gettime, .desc = "vDSO or syscall"  },
	};

	for (sz = 0; sz < ARRAY_SIZE(tf); sz++) {

		memset(&spec, 0, sizeof(struct timespec));

		TEST(tf[sz].func(tc[i].clktype, &spec));

		if (TST_RET == -1) {

			/* errors: allow unsupported clock types */

			if (tc[i].allow_inval && TST_ERR == EINVAL) {

				tst_res(TPASS, "clock_gettime(2): unsupported "
						"clock %s (%s) failed as "
						"expected",
						tst_clock_name(tc[i].clktype),
						tf[sz].desc);

			} else {

				tst_res(TFAIL | TTERRNO, "clock_gettime(2): "
						"clock %s (%s) failed "
						"unexpectedly",
						tst_clock_name(tc[i].clktype),
						tf[sz].desc);
			}

		} else {

			/* success: also check if timespec was changed */

			if (check_spec(&spec)) {
				tst_res(TPASS, "clock_gettime(2): clock %s "
						"(%s) passed",
						tst_clock_name(tc[i].clktype),
						tf[sz].desc);
			} else {

				tst_res(TFAIL, "clock_gettime(2): clock %s "
						"(%s) passed, unchanged "
						"timespec",
						tst_clock_name(tc[i].clktype),
						tf[sz].desc);
			}
		}
	}
}

static struct tst_test test = {
	.test = verify_clock_gettime,
	.tcnt = ARRAY_SIZE(tc),
	.needs_root = 1,
};
