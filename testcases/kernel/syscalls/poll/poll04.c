// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 Jinseok Kim <always.starving0@gmail.com>
 */

/*\
 * Check that poll() reports POLLNVAL for invalid file descriptors.
 */
#include <unistd.h>
#include <errno.h>
#include <sys/poll.h>

#include "tst_test.h"

static int fds[2];
static int invalid_fd;

static void verify_pollnval(void)
{
	struct pollfd pfd = {
		.fd = invalid_fd, .events = POLLIN,
	};

	TEST(poll(&pfd, 1, 0));

	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "poll() failed");
		return;
	}

	if (TST_RET != 1) {
		tst_res(TFAIL, "Unexpected poll() return value %ld", TST_RET);
		return;
	}

	TST_EXP_EXPR(pfd.revents & POLLNVAL);
	TST_EXP_EXPR((pfd.revents & ~POLLNVAL) == 0);

	tst_res(TPASS, "poll() reported POLLNVAL");
}

static void setup(void)
{
	SAFE_PIPE(fds);

	invalid_fd = fds[0];
	SAFE_CLOSE(fds[0]);
}

static void cleanup(void)
{
	if (fds[0] > 0)
		SAFE_CLOSE(fds[0]);

	if (fds[1] > 0)
		SAFE_CLOSE(fds[1]);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_pollnval,
};
