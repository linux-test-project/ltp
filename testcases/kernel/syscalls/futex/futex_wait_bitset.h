// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 */

#define THRESHOLD_US 100000
#define DEFAULT_TIMEOUT_US 100010

static void verify_futex_wait_bitset(long long wait_us, clock_t clk_id)
{
	struct timespec start, to, end;
	futex_t futex = FUTEX_INITIALIZER;
	u_int32_t bitset = 0xffffffff;
	int flags = clk_id == CLOCK_REALTIME ? FUTEX_CLOCK_REALTIME : 0;

	tst_res(TINFO, "testing futex_wait_bitset() timeout with %s",
		clk_id == CLOCK_REALTIME ? "CLOCK_REALTIME" : "CLOCK_MONOTONIC");

	clock_gettime(clk_id, &start);
	to = tst_timespec_add_us(start, wait_us);

	TEST(futex_wait_bitset(&futex, futex, &to, bitset, flags));

	clock_gettime(clk_id, &end);

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

	if (tst_timespec_lt(end, to)) {
		tst_res(TFAIL,
			"futex_wait_bitset() woken up prematurely %llius, expected %llius",
			tst_timespec_diff_us(end, start), wait_us);
		return;
	}

	if (tst_timespec_diff_us(end, to) > THRESHOLD_US) {
		tst_res(TFAIL,
			"futex_wait_bitset() waited too long %llius, expected %llius",
			tst_timespec_diff_us(end, start), wait_us);
		return;
	}

	tst_res(TPASS, "futex_wait_bitset() waited %llius, expected %llius",
		tst_timespec_diff_us(end, start), wait_us);
}

static void setup(void)
{
	tst_timer_check(USE_CLOCK);
}

static void run(void)
{
	verify_futex_wait_bitset(DEFAULT_TIMEOUT_US, USE_CLOCK);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
};
