/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>

#include "tst_timer_test.h"

int sample_fn(int clk_id, long long usec)
{
	fd_set readfds;
	struct timespec tv = tst_us_to_timespec(usec);

	FD_ZERO(&readfds);
	FD_SET(0, &readfds);

	tst_timer_start(clk_id);
	TEST(pselect(0, &readfds, NULL, NULL, &tv, NULL));
	tst_timer_stop();
	tst_timer_sample();

	if (TEST_RETURN != 0) {
		tst_res(TFAIL | TTERRNO,
			"pselect() returned %li on timeout", TEST_RETURN);
		return 1;
	}

	return 0;
}

static struct tst_test test = {
	.scall = "pselect()",
	.sample = sample_fn,
};
