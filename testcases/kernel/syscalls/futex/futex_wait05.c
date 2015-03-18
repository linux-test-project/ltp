/*
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
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

#include "test.h"
#include "futextest.h"

#define TRESHOLD_US 100000

const char *TCID="futex_wait05";
const int TST_TOTAL=1;

static void verify_futex_wait(clock_t clk_id, int fflags)
{
	struct timespec to = {.tv_sec = 0, .tv_nsec = 100010000};
	futex_t futex = FUTEX_INITIALIZER;

	tst_timer_start(clk_id);
	TEST(futex_wait(&futex, futex, &to, fflags));
	tst_timer_stop();

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "futex_wait() returned %li, expected -1",
		         TEST_RETURN);
		return;
	}

	if (TEST_ERRNO != ETIMEDOUT) {

		tst_resm(TFAIL | TTERRNO, "expected errno=%s",
		         tst_strerrno(ETIMEDOUT));
		return;
	}

	if (tst_timespec_lt(tst_timer_elapsed(), to)) {
		tst_resm(TFAIL,
		         "futex_wait() woken up prematurely %llius, expected %llius",
			 tst_timer_elapsed_us(), tst_timespec_to_us(to));
		return;
	}

	if (tst_timespec_diff_us(tst_timer_elapsed(), to) > TRESHOLD_US) {
		tst_resm(TFAIL,
		         "futex_wait() waited too long %llius, expected %llius",
			 tst_timer_elapsed_us(), tst_timespec_to_us(to));
		return;
	}

	tst_resm(TPASS, "futex_wait() waited %llius, expected %llius",
	         tst_timer_elapsed_us(), tst_timespec_to_us(to));
}

int main(int argc, char *argv[])
{
	int lc;

	tst_timer_check(CLOCK_MONOTONIC);

	tst_parse_opts(argc, argv, NULL, NULL);

	for (lc = 0; TEST_LOOPING(lc); lc++)
		verify_futex_wait(CLOCK_MONOTONIC, 0);

	tst_exit();
}
