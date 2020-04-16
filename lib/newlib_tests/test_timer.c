// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Tests for include/tst_timer.h
 */

#include "tst_test.h"
#include "tst_timer.h"

#define VAL_MS 1001
#define VAL_US 1001000

static void test_diff(enum tst_ts_type type)
{
	struct tst_ts ts1, ts2;
	long long diff;

	ts1 = tst_ts_from_ms(type, VAL_MS);
	ts2 = tst_ts_from_us(type, VAL_US);

	diff = tst_ts_diff_ns(ts1, ts2);

	if (diff == 0)
		tst_res(TPASS, "ns_diff = 0");
	else
		tst_res(TFAIL, "ns_diff = %lli", diff);

	diff = tst_ts_diff_ns(ts1, ts2);

	if (diff == 0)
		tst_res(TPASS, "us_diff = 0");
	else
		tst_res(TFAIL, "us_diff = %lli", diff);

	diff = tst_ts_diff_ms(ts1, ts2);

	if (diff == 0)
		tst_res(TPASS, "ms_diff = 0");
	else
		tst_res(TFAIL, "ms_diff = %lli", diff);
}

static void test_lt(enum tst_ts_type type)
{
	struct tst_ts ts1, ts2;

	ts1 = tst_ts_from_ms(type, VAL_MS);
	ts2 = tst_ts_from_us(type, VAL_US + 1);

	if (tst_ts_lt(ts1, ts2))
		tst_res(TPASS, "ts1 < ts2");
	else
		tst_res(TFAIL, "ts1 >= ts2");

	ts1 = tst_ts_add_us(ts1, 1);

	if (tst_ts_lt(ts1, ts2))
		tst_res(TFAIL, "ts1 < ts2");
	else
		tst_res(TPASS, "ts1 >= ts2");

	ts1 = tst_ts_add_us(ts1, 1);

	if (tst_ts_lt(ts1, ts2))
		tst_res(TFAIL, "ts1 < ts2");
	else
		tst_res(TPASS, "ts1 >= ts2");
}

static void test_add_sub(enum tst_ts_type type)
{
	struct tst_ts ts;

	ts = tst_ts_from_ns(type, 999999000);
	ts = tst_ts_add_us(ts, 1);

	long long sec = tst_ts_get_sec(ts);
	long long nsec = tst_ts_get_nsec(ts);

	/* Check that result was normalized */
	if (sec != 1 || nsec != 0)
		tst_res(TFAIL, "sec = %lli, nsec = %lli", sec, nsec);
	else
		tst_res(TPASS, "sec = %lli, nsec = %lli", sec, nsec);

	ts = tst_ts_from_ms(type, 1000);
	ts = tst_ts_sub_us(ts, 1);

	sec = tst_ts_get_sec(ts);
	nsec = tst_ts_get_nsec(ts);

	/* Check that result was normalized */
	if (sec != 0 || nsec != 999999000)
		tst_res(TFAIL, "sec = %lli, nsec = %lli", sec, nsec);
	else
		tst_res(TPASS, "sec = %lli, nsec = %lli", sec, nsec);
}

static void do_test(unsigned int n)
{
	tst_res(TINFO, "Testing with type = %i", n);
	test_diff(n);
	test_lt(n);
	test_add_sub(n);
}

static struct tst_test test = {
	.test = do_test,
	.tcnt = 3,
};
