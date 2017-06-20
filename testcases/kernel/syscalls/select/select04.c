/*
 * Copyright (C) 2015-2017 Cyril Hrubis <chrubis@suse.cz>
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
 * Check that select() timeouts correctly.
 */
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>

#include "tst_timer_test.h"

static int fds[2];

static int sample_fn(int clk_id, long long usec)
{
	struct timeval timeout = tst_us_to_timeval(usec);
	fd_set sfds;

	FD_ZERO(&sfds);

	FD_SET(fds[0], &sfds);

	tst_timer_start(clk_id);
	TEST(select(1, &sfds, NULL, NULL, &timeout));
	tst_timer_stop();
	tst_timer_sample();

	if (TEST_RETURN != 0) {
		tst_res(TFAIL | TTERRNO, "select() returned %li", TEST_RETURN);
		return 1;
	}

	return 0;
}

static void setup(void)
{
	SAFE_PIPE(fds);
}

static void cleanup(void)
{
	if (fds[0] > 0)
		SAFE_CLOSE(fds[0]);

	if (fds[1] > 0)
		SAFE_CLOSE(fds[1]);
}

static struct tst_test test = {
	.tid = "select()",
	.sample = sample_fn,
	.setup = setup,
	.cleanup = cleanup,
};
