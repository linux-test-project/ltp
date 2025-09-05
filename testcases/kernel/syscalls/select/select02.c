// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2015-2017 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Check that :man2:`select` timeouts correctly.
 */

#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>

#include "tst_timer_test.h"

#include "select_var.h"

static int fds[2];

static int sample_fn(int clk_id, long long usec)
{
	struct timeval timeout = tst_us_to_timeval(usec);
	fd_set sfds;

	FD_ZERO(&sfds);

	FD_SET(fds[0], &sfds);

	tst_timer_start(clk_id);
	TEST(do_select(1, &sfds, NULL, NULL, &timeout));
	tst_timer_stop();
	tst_timer_sample();

	if (TST_RET != 0) {
		tst_res(TFAIL | TTERRNO, "select() returned %li", TST_RET);
		return 1;
	}

	return 0;
}

static void setup(void)
{
	select_info();

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
	.scall = "select()",
	.sample = sample_fn,
	.setup = setup,
	.test_variants = TEST_VARIANTS,
	.cleanup = cleanup,
};
