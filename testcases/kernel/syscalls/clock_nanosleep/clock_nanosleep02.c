// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Test Description:
 *  clock_nanosleep() should return with value 0 and the process should be
 *  suspended for time specified by timespec structure.
 */

#include <errno.h>
#include "tst_timer_test.h"

int sample_fn(int clk_id, long long usec)
{
	struct timespec t = tst_timespec_from_us(usec);

	tst_timer_start(clk_id);
	TEST(clock_nanosleep(clk_id, 0, &t, NULL));
	tst_timer_stop();
	tst_timer_sample();

	if (TST_RET != 0) {
		tst_res(TFAIL | TERRNO,
			"nanosleep() returned %li", TST_RET);
		return 1;
	}

	return 0;
}

static struct tst_test test = {
	.scall = "clock_nanosleep()",
	.sample = sample_fn,
};
