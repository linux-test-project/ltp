// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Test for expiration timer the test should run for roughly 5 seconds
 * when executed as time ./tst_expiration_timer
 */

#include "tst_test.h"
#include "tst_timer.h"

static void do_test(void)
{
	tst_timer_start(CLOCK_MONOTONIC);

	while (!tst_timer_expired_ms(5000))
		usleep(1);

	tst_res(TPASS, "All done!");
}

static void setup(void)
{
	tst_timer_check(CLOCK_MONOTONIC);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = do_test,
};
