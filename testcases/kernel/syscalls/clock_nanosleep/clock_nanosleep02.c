/*
 * Copyright (C) 2017 Cyril Hrubis <chrubis@suse.cz>
 *
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
 * along with this program;  if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
	struct timespec t = tst_us_to_timespec(usec);

	tst_timer_start(clk_id);
	TEST(clock_nanosleep(clk_id, 0, &t, NULL));
	tst_timer_stop();
	tst_timer_sample();

	if (TEST_RETURN != 0) {
		tst_res(TFAIL | TERRNO,
			"nanosleep() returned %li", TEST_RETURN);
		return 1;
	}

	return 0;
}

static struct tst_test test = {
	.scall = "clock_nanosleep()",
	.sample = sample_fn,
};
