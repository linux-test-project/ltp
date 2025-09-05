// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Linaro Limited. All rights reserved.
 * Author: Rafael David Tinoco <rafael.tinoco@linaro.org>
 */

/*\
 * Bad argument tests for clock_gettime(2) on multiple clocks:
 *
 * #. MAX_CLOCKS
 * #. MAX_CLOCKS + 1
 * #. CLOCK_REALTIME
 * #. CLOCK_MONOTONIC
 * #. CLOCK_PROCESS_CPUTIME_ID
 * #. CLOCK_THREAD_CPUTIME_ID
 * #. CLOCK_REALTIME_COARSE
 * #. CLOCK_MONOTONIC_COARSE
 * #. CLOCK_MONOTONIC_RAW
 * #. CLOCK_BOOTTIME
 */

#include "config.h"
#include "time64_variants.h"
#include "tst_timer.h"
#include "tst_safe_clocks.h"

static void *bad_addr;
static clockid_t max_clocks1;
static clockid_t max_clocks2;
static clockid_t clk_realtime = CLOCK_REALTIME;
static clockid_t clk_monotonic = CLOCK_MONOTONIC;
static clockid_t clk_process_cputime_id = CLOCK_PROCESS_CPUTIME_ID;
static clockid_t clk_thread_cputime_id = CLOCK_THREAD_CPUTIME_ID;
static clockid_t clk_realtime_coarse = CLOCK_REALTIME_COARSE;
static clockid_t clk_monotonic_coarse = CLOCK_MONOTONIC_COARSE;
static clockid_t clk_monotonic_raw = CLOCK_MONOTONIC_RAW;
static clockid_t clk_boottime = CLOCK_BOOTTIME;

struct test_case {
	clockid_t *clktype;
	int exp_err;
	int allow_inval;
};

static struct test_case tc[] = {
	{
	 .clktype = &max_clocks1,
	 .exp_err = EINVAL,
	 },
	{
	 .clktype = &max_clocks2,
	 .exp_err = EINVAL,
	 },
	/*
	 * Different POSIX clocks have different (*clock_get)() handlers.
	 * It justifies testing EFAULT for all.
	 */
	{
	 .clktype = &clk_realtime,
	 .exp_err = EFAULT,
	 },
	{
	 .clktype = &clk_monotonic,
	 .exp_err = EFAULT,
	 },
	{
	 .clktype = &clk_process_cputime_id,
	 .exp_err = EFAULT,
	 },
	{
	 .clktype = &clk_thread_cputime_id,
	 .exp_err = EFAULT,
	 },
	{
	 .clktype = &clk_realtime_coarse,
	 .exp_err = EFAULT,
	 .allow_inval = 1,
	 },
	{
	 .clktype = &clk_monotonic_coarse,
	 .exp_err = EFAULT,
	 .allow_inval = 1,
	 },
	{
	 .clktype = &clk_monotonic_raw,
	 .exp_err = EFAULT,
	 .allow_inval = 1,
	 },
	{
	 .clktype = &clk_boottime,
	 .exp_err = EFAULT,
	 .allow_inval = 1,
	 },
};

static struct tst_ts spec;

/*
 * bad pointer w/ libc causes SIGSEGV signal, call syscall directly
 */
static struct time64_variants variants[] = {
#if (__NR_clock_gettime != __LTP__NR_INVALID_SYSCALL)
	{ .clock_gettime = sys_clock_gettime, .ts_type = TST_KERN_OLD_TIMESPEC, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_clock_gettime64 != __LTP__NR_INVALID_SYSCALL)
	{ .clock_gettime = sys_clock_gettime64, .ts_type = TST_KERN_TIMESPEC, .desc = "syscall time64 with kernel spec"},
#endif
};

static void setup(void)
{
	tst_res(TINFO, "Testing variant: %d: %s", tst_variant, variants[tst_variant].desc);

	bad_addr = tst_get_bad_addr(NULL);

	max_clocks1 = tst_get_max_clocks();
	max_clocks2 = max_clocks1 + 1;
}

static void verify_clock_gettime(unsigned int i)
{
	struct time64_variants *tv = &variants[tst_variant];
	void *ts;

	/* bad pointer cases */
	if (tc[i].exp_err == EFAULT) {
		ts = bad_addr;
	} else {
		spec.type = tv->ts_type;
		ts = tst_ts_get(&spec);
	}

	TEST(tv->clock_gettime(*tc[i].clktype, ts));

	if (TST_RET != -1) {
		tst_res(TFAIL, "clock_gettime(2): clock %s passed unexpectedly",
			tst_clock_name(*tc[i].clktype));
		return;
	}

	if ((tc[i].exp_err == TST_ERR) ||
	    (tc[i].allow_inval && TST_ERR == EINVAL)) {
		tst_res(TPASS | TTERRNO, "clock_gettime(2): clock %s failed as expected",
			tst_clock_name(*tc[i].clktype));
	} else {
		tst_res(TFAIL | TTERRNO, "clock_gettime(2): clock %s failed unexpectedly",
			tst_clock_name(*tc[i].clktype));
	}
}

static struct tst_test test = {
	.test = verify_clock_gettime,
	.tcnt = ARRAY_SIZE(tc),
	.test_variants = ARRAY_SIZE(variants),
	.setup = setup,
	.needs_root = 1,
};
