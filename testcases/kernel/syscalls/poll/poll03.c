// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 Jinseok Kim <always.starving0@gmail.com>
 */

/*\
 * Check that poll() reports POLLHUP on a pipe read end
 * after the write end has been closed.
 */
#include <unistd.h>
#include <errno.h>
#include <sys/poll.h>

#include "tst_test.h"

static int fds[2];

static void verify_pollhup(void)
{
	struct pollfd pfd = {
		.fd = fds[0], .events = POLLIN,
	};

	TEST(poll(&pfd, 1, -1));

	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "poll() failed");
		return;
	}

	if (TST_RET != 1) {
		tst_res(TFAIL, "Unexpected poll() return value %ld", TST_RET);
		return;
	}

	TST_EXP_EXPR(pfd.revents & POLLHUP);
	TST_EXP_EXPR((pfd.revents & ~POLLHUP) == 0);

	tst_res(TPASS, "poll() reported POLLHUP");
}

static void setup(void)
{
	SAFE_PIPE(fds);
	SAFE_CLOSE(fds[1]);
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
	.test_all = verify_pollhup,
};
