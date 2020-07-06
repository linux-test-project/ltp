// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 *
 * 1. Block on a bitset futex and wait for timeout, the difference between
 *    normal futex and bitset futex is that that the later have absolute timeout.
 * 2. Check that the futex waited for expected time.
 */

#include "tst_test.h"
#include "tst_timer.h"
#include "futextest.h"

#define THRESHOLD_US 100000
#define DEFAULT_TIMEOUT_US 100010

static struct test_case_t {
	clockid_t clk_id;
} tcases[] = {
	{ CLOCK_MONOTONIC },
	{ CLOCK_REALTIME }
};

static struct test_variants {
	enum futex_fn_type fntype;
	enum tst_ts_type tstype;
	int (*gettime)(clockid_t clk_id, void *ts);
	char *desc;
} variants[] = {
#if (__NR_futex != __LTP__NR_INVALID_SYSCALL)
	{ .fntype = FUTEX_FN_FUTEX, .tstype = TST_KERN_OLD_TIMESPEC, .gettime = sys_clock_gettime, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_futex_time64 != __LTP__NR_INVALID_SYSCALL)
	{ .fntype = FUTEX_FN_FUTEX64, .tstype = TST_KERN_TIMESPEC, .gettime = sys_clock_gettime64, .desc = "syscall time64 with kernel spec"},
#endif
};

static void verify_futex_wait_bitset(long long wait_us, clock_t clk_id)
{
	struct test_variants *tv = &variants[tst_variant];
	struct tst_ts start, to, end;
	futex_t futex = FUTEX_INITIALIZER;
	u_int32_t bitset = 0xffffffff;
	int flags = clk_id == CLOCK_REALTIME ? FUTEX_CLOCK_REALTIME : 0;

	start.type = end.type = to.type = tv->tstype;

	tst_res(TINFO, "testing futex_wait_bitset() timeout with %s",
		clk_id == CLOCK_REALTIME ? "CLOCK_REALTIME" : "CLOCK_MONOTONIC");

	tv->gettime(clk_id, tst_ts_get(&start));
	to = tst_ts_add_us(start, wait_us);

	TEST(futex_wait_bitset(tv->fntype, &futex, futex, &to, bitset, flags));

	tv->gettime(clk_id, tst_ts_get(&end));

	if (TST_RET != -1) {
		tst_res(TFAIL, "futex_wait_bitset() returned %li, expected -1",
			TST_RET);
		return;
	}

	if (TST_ERR == ENOSYS) {
		tst_res(TCONF,
			"In this kernel, futex() does not support FUTEX_WAIT_BITSET operation");
		return;
	}

	if (TST_ERR != ETIMEDOUT) {
		tst_res(TFAIL | TTERRNO, "expected %s",
			tst_strerrno(ETIMEDOUT));
		return;
	}

	if (tst_ts_lt(end, to)) {
		tst_res(TFAIL,
			"futex_wait_bitset() woken up prematurely %llius, expected %llius",
			tst_ts_diff_us(end, start), wait_us);
		return;
	}

	if (tst_ts_diff_us(end, to) > THRESHOLD_US) {
		tst_res(TFAIL,
			"futex_wait_bitset() waited too long %llius, expected %llius",
			tst_ts_diff_us(end, start), wait_us);
		return;
	}

	tst_res(TPASS, "futex_wait_bitset() waited %llius, expected %llius",
		tst_ts_diff_us(end, start), wait_us);
}

static void run(unsigned int n)
{
	verify_futex_wait_bitset(DEFAULT_TIMEOUT_US, tcases[n].clk_id);
}

static void setup(void)
{
	struct test_variants *tv = &variants[tst_variant];

	tst_res(TINFO, "Testing variant: %s", tv->desc);
	futex_supported_by_kernel(tv->fntype);
}

static struct tst_test test = {
	.setup = setup,
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.test_variants = ARRAY_SIZE(variants),
};
