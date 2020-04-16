// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2015-2017 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * 1. Block on a futex and wait for timeout.
 * 2. Check that the futex waited for expected time.
 */

#include <errno.h>

#include "tst_timer_test.h"
#include "futextest.h"

int sample_fn(int clk_id, long long usec)
{
	struct timespec to = tst_timespec_from_us(usec);
	futex_t futex = FUTEX_INITIALIZER;

	tst_timer_start(clk_id);
	TEST(futex_wait(&futex, futex, &to, 0));
	tst_timer_stop();
	tst_timer_sample();

	if (TST_RET != -1) {
		tst_res(TFAIL, "futex_wait() returned %li, expected -1",
			TST_RET);
		return 1;
	}

	if (TST_ERR != ETIMEDOUT) {
		tst_res(TFAIL | TTERRNO, "expected errno=%s",
		        tst_strerrno(ETIMEDOUT));
		return 1;
	}

	return 0;
}

static struct tst_test test = {
	.scall = "futex_wait()",
	.sample = sample_fn,
};
