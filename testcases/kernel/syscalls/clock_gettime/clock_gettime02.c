// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Linaro Limited. All rights reserved.
 * Author: Rafael David Tinoco <rafael.tinoco@linaro.org>
 */
/*
 * Bad argument tests for clock_gettime(2) on multiple clocks:
 *
 *  1) MAX_CLOCKS
 *  2) MAX_CLOCKS + 1
 *  3) CLOCK_REALTIME
 *  4) CLOCK_MONOTONIC
 *  5) CLOCK_PROCESS_CPUTIME_ID
 *  6) CLOCK_THREAD_CPUTIME_ID
 *  7) CLOCK_REALTIME_COARSE
 *  8) CLOCK_MONOTONIC_COARSE
 *  9) CLOCK_MONOTONIC_RAW
 * 10) CLOCK_BOOTTIME
 */

#include "config.h"
#include "tst_timer.h"
#include "tst_safe_clocks.h"

static void *bad_addr;

struct test_case {
	clockid_t clktype;
	int exp_err;
	int allow_inval;
};

static struct test_case tc[] = {
	{
	 .clktype = MAX_CLOCKS,
	 .exp_err = EINVAL,
	 },
	{
	 .clktype = MAX_CLOCKS + 1,
	 .exp_err = EINVAL,
	 },
	/*
	 * Different POSIX clocks have different (*clock_get)() handlers.
	 * It justifies testing EFAULT for all.
	 */
	{
	 .clktype = CLOCK_REALTIME,
	 .exp_err = EFAULT,
	 },
	{
	 .clktype = CLOCK_MONOTONIC,
	 .exp_err = EFAULT,
	 },
	{
	 .clktype = CLOCK_PROCESS_CPUTIME_ID,
	 .exp_err = EFAULT,
	 },
	{
	 .clktype = CLOCK_THREAD_CPUTIME_ID,
	 .exp_err = EFAULT,
	 },
	{
	 .clktype = CLOCK_REALTIME_COARSE,
	 .exp_err = EFAULT,
	 .allow_inval = 1,
	 },
	{
	 .clktype = CLOCK_MONOTONIC_COARSE,
	 .exp_err = EFAULT,
	 .allow_inval = 1,
	 },
	{
	 .clktype = CLOCK_MONOTONIC_RAW,
	 .exp_err = EFAULT,
	 .allow_inval = 1,
	 },
	{
	 .clktype = CLOCK_BOOTTIME,
	 .exp_err = EFAULT,
	 .allow_inval = 1,
	 },
};

static struct tst_ts spec;

/*
 * bad pointer w/ libc causes SIGSEGV signal, call syscall directly
 */
static struct test_variants {
	int (*func)(clockid_t clk_id, void *ts);
	enum tst_ts_type type;
	char *desc;
} variants[] = {
#if (__NR_clock_gettime != __LTP__NR_INVALID_SYSCALL)
	{ .func = sys_clock_gettime, .type = TST_KERN_OLD_TIMESPEC, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_clock_gettime64 != __LTP__NR_INVALID_SYSCALL)
	{ .func = sys_clock_gettime64, .type = TST_KERN_TIMESPEC, .desc = "syscall time64 with kernel spec"},
#endif
};

static void setup(void)
{
	tst_res(TINFO, "Testing variant: %d: %s", tst_variant, variants[tst_variant].desc);

	bad_addr = tst_get_bad_addr(NULL);
}

static void verify_clock_gettime(unsigned int i)
{
	struct test_variants *tv = &variants[tst_variant];
	void *ts;

	/* bad pointer cases */
	if (tc[i].exp_err == EFAULT) {
		ts = bad_addr;
	} else {
		spec.type = tv->type;
		ts = tst_ts_get(&spec);
	}

	TEST(tv->func(tc[i].clktype, ts));

	if (TST_RET != -1) {
		tst_res(TFAIL, "clock_gettime(2): clock %s passed unexcpectedly",
			tst_clock_name(tc[i].clktype));
		return;
	}

	if ((tc[i].exp_err == TST_ERR) ||
	    (tc[i].allow_inval && TST_ERR == EINVAL)) {
		tst_res(TPASS | TTERRNO, "clock_gettime(2): clock %s failed as expected",
			tst_clock_name(tc[i].clktype));
	} else {
		tst_res(TFAIL | TTERRNO, "clock_gettime(2): clock %s failed unexpectedly",
			tst_clock_name(tc[i].clktype));
	}
}

static struct tst_test test = {
	.test = verify_clock_gettime,
	.tcnt = ARRAY_SIZE(tc),
	.test_variants = ARRAY_SIZE(variants),
	.setup = setup,
	.needs_root = 1,
};
