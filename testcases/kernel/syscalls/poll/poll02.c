// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2015-2017 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Check that poll() timeouts correctly.
 */
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/poll.h>

#include "tst_timer_test.h"

static int fds[2];

int sample_fn(int clk_id, long long usec)
{
	unsigned int sleep_ms = usec / 1000;

	struct pollfd pfds[] = {
		{.fd = fds[0], .events = POLLIN}
	};

	tst_timer_start(clk_id);
	TEST(poll(pfds, 1, sleep_ms));
	tst_timer_stop();
	tst_timer_sample();

	if (TST_RET != 0) {
		tst_res(TFAIL | TTERRNO, "poll() returned %li", TST_RET);
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
	.scall = "poll()",
	.sample = sample_fn,
	.setup = setup,
	.cleanup = cleanup,
};
