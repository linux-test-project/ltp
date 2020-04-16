// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 */

/*
 * Test Description:
 *  This is a timer sample test that timer slack is 200us.
 */

#include <errno.h>
#include <sys/prctl.h>
#include "lapi/prctl.h"
#include "tst_timer_test.h"

int sample_fn(int clk_id, long long usec)
{
	struct timespec t = tst_timespec_from_us(usec);

	tst_timer_start(clk_id);
	TEST(nanosleep(&t, NULL));
	tst_timer_stop();
	tst_timer_sample();

	if (TST_RET != 0) {
		tst_res(TFAIL | TTERRNO,
			"nanosleep() returned %li", TST_RET);
		return 1;
	}

	return 0;
}

static void setup(void)
{
	TEST(prctl(PR_SET_TIMERSLACK, 200000));
	if (TST_RET != 0)
		tst_brk(TBROK | TTERRNO,
			"prctl set timerslack 200us failed");
}

static struct tst_test test = {
	.setup = setup,
	.scall = "prctl()",
	.sample = sample_fn,
};
