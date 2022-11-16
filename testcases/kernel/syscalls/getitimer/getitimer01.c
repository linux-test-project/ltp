// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	03/2001 - Written by Wayne Boyer
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Check that a correct call to getitimer() succeeds.
 */

#include "tst_test.h"
#include "tst_safe_clocks.h"

#define SEC  100
#define USEC 10000

static struct timeval tv;
static struct itimerval *value;
static long jiffy;

static struct tcase {
	int which;
	char *des;
} tcases[] = {
	{ITIMER_REAL,    "ITIMER_REAL"},
	{ITIMER_VIRTUAL, "ITIMER_VIRTUAL"},
	{ITIMER_PROF,    "ITIMER_PROF"},
};

static void set_setitimer_value(int sec, int usec)
{
	value->it_value.tv_sec = sec;
	value->it_value.tv_usec = usec;
	value->it_interval.tv_sec = sec;
	value->it_interval.tv_usec = usec;
}

static void verify_getitimer(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	tst_res(TINFO, "tc->which = %s", tc->des);

	if (tc->which == ITIMER_REAL) {
		if (gettimeofday(&tv, NULL) == -1)
			tst_brk(TBROK | TERRNO, "gettimeofday(&tv1, NULL) failed");
		else
			tst_res(TINFO, "Test begin time: %ld.%lds", tv.tv_sec, tv.tv_usec);
	}

	TST_EXP_PASS(getitimer(tc->which, value));
	TST_EXP_EQ_LI(value->it_value.tv_sec, 0);
	TST_EXP_EQ_LI(value->it_value.tv_usec, 0);
	TST_EXP_EQ_LI(value->it_interval.tv_sec, 0);
	TST_EXP_EQ_LI(value->it_interval.tv_usec, 0);

	set_setitimer_value(SEC, USEC);
	TST_EXP_PASS(setitimer(tc->which, value, NULL));

	set_setitimer_value(0, 0);
	TST_EXP_PASS(getitimer(tc->which, value));

	TST_EXP_EQ_LI(value->it_interval.tv_sec, SEC);
	TST_EXP_EQ_LI(value->it_interval.tv_usec, USEC);

	tst_res(TINFO, "value->it_value.tv_sec=%ld, value->it_value.tv_usec=%ld",
			value->it_value.tv_sec, value->it_value.tv_usec);

	/*
	 * ITIMER_VIRTUAL and ITIMER_PROF timers always expire a
	 * TICK_NSEC (jiffy) afterward the elapsed time to make
	 * sure that at least time counters take effect.
	 */
	long margin = (tc->which == ITIMER_REAL) ? 0 : jiffy;

	if (value->it_value.tv_sec == SEC) {
		if (value->it_value.tv_usec < 0 ||
				value->it_value.tv_usec > USEC + margin)
			tst_brk(TFAIL, "value->it_value.tv_usec is out of range: %ld",
				value->it_value.tv_usec);
	} else {
		if (value->it_value.tv_sec < 0 ||
				value->it_value.tv_sec > SEC)
			tst_brk(TFAIL, "value->it_value.tv_sec is out of range: %ld",
				value->it_value.tv_sec);
	}

	tst_res(TPASS, "timer value is within the expected range");

	if (tc->which == ITIMER_REAL) {
		if (gettimeofday(&tv, NULL) == -1)
			tst_brk(TBROK | TERRNO, "gettimeofday(&tv1, NULL) failed");
		else
			tst_res(TINFO, "Test end time: %ld.%lds", tv.tv_sec, tv.tv_usec);
	}

	set_setitimer_value(0, 0);
	TST_EXP_PASS_SILENT(setitimer(tc->which, value, NULL));
}

static void setup(void)
{
	struct timespec time_res;

	SAFE_CLOCK_GETRES(CLOCK_MONOTONIC_COARSE, &time_res);
	jiffy = (time_res.tv_nsec + 999) / 1000;
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.test = verify_getitimer,
	.bufs = (struct tst_buffers[]) {
		{&value,  .size = sizeof(struct itimerval)},
		{}
	}
};
