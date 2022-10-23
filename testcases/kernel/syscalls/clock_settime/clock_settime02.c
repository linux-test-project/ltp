// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Linaro Limited. All rights reserved.
 * Author: Rafael David Tinoco <rafael.tinoco@linaro.org>
 */

/*
 * Basic tests for errors of clock_settime(2) on different clock types.
 */

#include "config.h"
#include "time64_variants.h"
#include "tst_timer.h"
#include "tst_safe_clocks.h"

#define DELTA_SEC 10

static void *bad_addr;

struct test_case {
	clockid_t type;
	int exp_err;
	int replace;
	long tv_sec;
	long tv_nsec;
};

struct test_case tc[] = {
	{				/* case 01: REALTIME: timespec NULL   */
	 .type = CLOCK_REALTIME,
	 .exp_err = EFAULT,
	 .replace = 1,
	 .tv_sec = 0,
	 .tv_nsec = 0,
	 },
	{				/* case 02: REALTIME: tv_sec = -1     */
	 .type = CLOCK_REALTIME,
	 .exp_err = EINVAL,
	 .replace = 1,
	 .tv_sec = -1,
	 .tv_nsec = 0,
	 },
	{				/* case 03: REALTIME: tv_nsec = -1    */
	 .type = CLOCK_REALTIME,
	 .exp_err = EINVAL,
	 .replace = 1,
	 .tv_sec = 0,
	 .tv_nsec = -1,
	 },
	{				/* case 04: REALTIME: tv_nsec = 1s+1  */
	 .type = CLOCK_REALTIME,
	 .exp_err = EINVAL,
	 .replace = 1,
	 .tv_sec = 0,
	 .tv_nsec = NSEC_PER_SEC + 1,
	 },
	{				/* case 05: MONOTONIC		      */
	 .type = CLOCK_MONOTONIC,
	 .exp_err = EINVAL,
	 },
	{				/* case 06: MAXCLOCK		      */
	 .type = MAX_CLOCKS,
	 .exp_err = EINVAL,
	 },
	{				/* case 07: MAXCLOCK+1		      */
	 .type = MAX_CLOCKS + 1,
	 .exp_err = EINVAL,
	 },
	/* Linux specific */
	{				/* case 08: CLOCK_MONOTONIC_COARSE    */
	 .type = CLOCK_MONOTONIC_COARSE,
	 .exp_err = EINVAL,
	 },
	{				/* case 09: CLOCK_MONOTONIC_RAW       */
	 .type = CLOCK_MONOTONIC_RAW,
	 .exp_err = EINVAL,
	 },
	{				/* case 10: CLOCK_BOOTTIME	      */
	 .type = CLOCK_BOOTTIME,
	 .exp_err = EINVAL,
	 },
	{				/* case 11: CLOCK_PROCESS_CPUTIME_ID  */
	 .type = CLOCK_PROCESS_CPUTIME_ID,
	 .exp_err = EINVAL,
	 },
	{				/* case 12: CLOCK_THREAD_CPUTIME_ID   */
	 .type = CLOCK_THREAD_CPUTIME_ID,
	 .exp_err = EINVAL,
	 },
};

static struct tst_ts spec;

static struct time64_variants variants[] = {
#if (__NR_clock_settime != __LTP__NR_INVALID_SYSCALL)
	{ .clock_gettime = sys_clock_gettime, .clock_settime = sys_clock_settime, .ts_type = TST_KERN_OLD_TIMESPEC, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_clock_settime64 != __LTP__NR_INVALID_SYSCALL)
	{ .clock_gettime = sys_clock_gettime64, .clock_settime = sys_clock_settime64, .ts_type = TST_KERN_TIMESPEC, .desc = "syscall time64 with kernel spec"},
#endif
};

static void setup(void)
{
	tst_res(TINFO, "Testing variant: %s", variants[tst_variant].desc);

	bad_addr = tst_get_bad_addr(NULL);
}

static void verify_clock_settime(unsigned int i)
{
	struct time64_variants *tv = &variants[tst_variant];
	void *ts;

	spec.type = tv->ts_type;

	if (tc[i].replace == 0) {
		TEST(tv->clock_gettime(CLOCK_REALTIME, tst_ts_get(&spec)));
		if (TST_RET == -1) {
			tst_res(TFAIL | TTERRNO, "clock_gettime(2) failed for clock %s",
				tst_clock_name(CLOCK_REALTIME));
			return;
		}

		/* add 1 sec to wall clock */
		spec = tst_ts_add_us(spec, 1000000);
	} else {
		/* use given time spec */
		tst_ts_set_sec(&spec, tc[i].tv_sec);
		tst_ts_set_nsec(&spec, tc[i].tv_nsec);
	}

	/* bad pointer case */
	if (tc[i].exp_err == EFAULT)
		ts = bad_addr;
	else
		ts = tst_ts_get(&spec);

	TEST(tv->clock_settime(tc[i].type, ts));

	if (TST_RET != -1) {
		tst_res(TFAIL | TTERRNO, "clock_settime(2): clock %s passed unexpectedly, expected %s",
			tst_clock_name(tc[i].type),
			tst_strerrno(tc[i].exp_err));
		return;
	}

	if (tc[i].exp_err == TST_ERR) {
		tst_res(TPASS | TTERRNO, "clock_settime(%s): failed as expected",
			tst_clock_name(tc[i].type));
		return;
	}

	tst_res(TFAIL | TTERRNO, "clock_settime(2): clock %s " "expected to fail with %s",
		tst_clock_name(tc[i].type), tst_strerrno(tc[i].exp_err));
}

static struct tst_test test = {
	.test = verify_clock_settime,
	.test_variants = ARRAY_SIZE(variants),
	.setup = setup,
	.tcnt = ARRAY_SIZE(tc),
	.needs_root = 1,
	.restore_wallclock = 1,
};
