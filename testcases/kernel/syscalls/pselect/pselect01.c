// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>

#include "tst_timer_test.h"

int sample_fn(int clk_id, long long usec)
{
	fd_set readfds;
	struct timespec tv = tst_timespec_from_us(usec);

	FD_ZERO(&readfds);
	FD_SET(0, &readfds);

	tst_timer_start(clk_id);
	TEST(pselect(0, &readfds, NULL, NULL, &tv, NULL));
	tst_timer_stop();
	tst_timer_sample();

	if (TST_RET != 0) {
		tst_res(TFAIL | TTERRNO,
			"pselect() returned %li on timeout", TST_RET);
		return 1;
	}

	return 0;
}

static struct tst_test test = {
	.scall = "pselect()",
	.sample = sample_fn,
};
