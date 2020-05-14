// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Linaro Limited. All rights reserved.
 * Author: Rafael David Tinoco <rafael.tinoco@linaro.org>
 */

/*
 * Basic test for clock_settime(2) on REALTIME clock:
 *
 *      1) advance DELTA_SEC seconds
 *      2) go backwards DELTA_SEC seconds
 *
 * Restore wall clock at the end of test.
 */

#include "config.h"
#include "tst_timer.h"
#include "tst_safe_clocks.h"

#define DELTA_SEC 10
#define DELTA_US (long long) (DELTA_SEC * 1000000)
#define DELTA_EPS (long long) (DELTA_US * 0.1)

static struct tst_ts *begin, *change, *end;

static struct test_variants {
	int (*gettime)(clockid_t clk_id, void *ts);
	int (*settime)(clockid_t clk_id, void *ts);
	enum tst_ts_type type;
	char *desc;
} variants[] = {
	{ .gettime = libc_clock_gettime, .settime = libc_clock_settime, .type = TST_LIBC_TIMESPEC, .desc = "vDSO or syscall with libc spec"},

#if (__NR_clock_settime != __LTP__NR_INVALID_SYSCALL)
	{ .gettime = sys_clock_gettime, .settime = sys_clock_settime, .type = TST_KERN_OLD_TIMESPEC, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_clock_settime64 != __LTP__NR_INVALID_SYSCALL)
	{ .gettime = sys_clock_gettime64, .settime = sys_clock_settime64, .type = TST_KERN_TIMESPEC, .desc = "syscall time64 with kernel spec"},
#endif
};

static void setup(void)
{
	begin->type = change->type = end->type = variants[tst_variant].type;
	tst_res(TINFO, "Testing variant: %s", variants[tst_variant].desc);
}

static void do_clock_gettime(struct test_variants *tv, struct tst_ts *ts)
{
	int ret;

	ret = tv->gettime(CLOCK_REALTIME, tst_ts_get(ts));
	if (ret == -1)
		tst_brk(TBROK | TERRNO, "clock_settime(CLOCK_REALTIME) failed");
}

static void verify_clock_settime(void)
{
	struct test_variants *tv = &variants[tst_variant];
	long long elapsed;

	/* test 01: move forward */
	do_clock_gettime(tv, begin);

	*change = tst_ts_add_us(*begin, DELTA_US);

	TEST(tv->settime(CLOCK_REALTIME, tst_ts_get(change)));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "clock_settime(2) failed for clock %s",
			tst_clock_name(CLOCK_REALTIME));
		return;
	}

	do_clock_gettime(tv, end);

	elapsed = tst_ts_diff_us(*end, *begin);

	if (elapsed >= DELTA_US && elapsed < (DELTA_US + DELTA_EPS))
		tst_res(TPASS, "clock_settime(2): was able to advance time");
	else
		tst_res(TFAIL, "clock_settime(2): could not advance time");

	/* test 02: move backward */
	do_clock_gettime(tv, begin);

	*change = tst_ts_sub_us(*begin, DELTA_US);

	TEST(tv->settime(CLOCK_REALTIME, tst_ts_get(change)));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "clock_settime(2) failed for clock %s",
			tst_clock_name(CLOCK_REALTIME));
		return;
	}

	do_clock_gettime(tv, end);

	elapsed = tst_ts_diff_us(*end, *begin);

	if (~(elapsed) <= DELTA_US && ~(elapsed) > (DELTA_US - DELTA_EPS))
		tst_res(TPASS, "clock_settime(2): was able to recede time");
	else
		tst_res(TFAIL, "clock_settime(2): could not recede time");
}

static struct tst_test test = {
	.test_all = verify_clock_settime,
	.test_variants = ARRAY_SIZE(variants),
	.setup = setup,
	.needs_root = 1,
	.restore_wallclock = 1,
	.bufs = (struct tst_buffers []) {
		{&begin, .size = sizeof(*begin)},
		{&change, .size = sizeof(*change)},
		{&end, .size = sizeof(*end)},
		{},
	}
};
