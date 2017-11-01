/*
 * Copyright (C) 2015-2017 Cyril Hrubis <chrubis@suse.cz>
 *
 * Licensed under the GNU GPLv2 or later.
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
	struct timespec to = tst_us_to_timespec(usec);
	futex_t futex = FUTEX_INITIALIZER;

	tst_timer_start(clk_id);
	TEST(futex_wait(&futex, futex, &to, 0));
	tst_timer_stop();
	tst_timer_sample();

	if (TEST_RETURN != -1) {
		tst_res(TFAIL, "futex_wait() returned %li, expected -1",
		         TEST_RETURN);
		return 1;
	}

	if (TEST_ERRNO != ETIMEDOUT) {
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
