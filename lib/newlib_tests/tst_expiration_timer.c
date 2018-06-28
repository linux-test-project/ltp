/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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
