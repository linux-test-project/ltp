// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Verify that :manpage:`epoll_wait(2)` works with nested epoll instances up to
 * the maximum nesting depth of 5 allowed by the kernel (EP_MAX_NESTS).
 *
 * [Algorithm]
 *
 * - Create a pipe.
 * - Build a chain of epoll instances of the given depth where the innermost
 *   monitors the pipe read end and each subsequent one monitors the previous.
 * - Write data to the pipe.
 * - Call :manpage:`epoll_wait(2)` on the outermost epoll instance and verify
 *   it reports EPOLLIN.
 * - Walk the chain inward calling epoll_wait on each level and verify that
 *   every level reports EPOLLIN on the expected fd.
 * - Read the data from the pipe to drain it.
 */

#include <sys/epoll.h>

#include "tst_epoll.h"
#include "tst_test.h"

#define MAX_DEPTH 5

static int epfds[MAX_DEPTH];
static int fds[2] = {-1, -1};

static void run(unsigned int n)
{
	struct epoll_event ev;
	int prev_fd, i;
	char buf;
	int depth = n + 2;

	prev_fd = fds[0];

	for (i = 0; i < depth; i++) {
		epfds[i] = SAFE_EPOLL_CREATE1(0);

		ev.events = EPOLLIN;
		ev.data.fd = prev_fd;

		SAFE_EPOLL_CTL(epfds[i], EPOLL_CTL_ADD, prev_fd, &ev);

		prev_fd = epfds[i];
	}

	SAFE_WRITE(SAFE_WRITE_ALL, fds[1], "x", 1);

	/* Walk from outermost to innermost verifying each level */
	for (i = depth - 1; i >= 0; i--) {
		int expected_fd = (i > 0) ? epfds[i - 1] : fds[0];

		TEST(epoll_wait(epfds[i], &ev, 1, 1000));
		if (TST_RET != 1) {
			tst_res(TFAIL | TTERRNO,
				"depth %d/%d: epoll_wait() returned %li, expected 1",
				depth - i, depth, TST_RET);
			goto end;
		}

		if (ev.data.fd != expected_fd) {
			tst_res(TFAIL,
				"depth %d/%d: data.fd %d, expected %d",
				depth - i, depth,
				ev.data.fd, expected_fd);
			goto end;
		}

		if (!(ev.events & EPOLLIN)) {
			tst_res(TFAIL,
				"depth %d/%d: events %x, expected EPOLLIN",
				depth - i, depth, ev.events);
			goto end;
		}
	}

	tst_res(TPASS, "epoll_wait() with %d nested levels", depth);

end:
	SAFE_READ(1, fds[0], &buf, 1);

	for (i = depth - 1; i >= 0; i--) {
		if (epfds[i] != -1)
			SAFE_CLOSE(epfds[i]);
	}
}

static void setup(void)
{
	int i;

	for (i = 0; i < MAX_DEPTH; i++)
		epfds[i] = -1;

	SAFE_PIPE(fds);
}

static void cleanup(void)
{
	int i;

	for (i = 0; i < MAX_DEPTH; i++) {
		if (epfds[i] != -1)
			SAFE_CLOSE(epfds[i]);
	}

	SAFE_CLOSE(fds[0]);
	SAFE_CLOSE(fds[1]);
}

static struct tst_test test = {
	.test = run,
	.setup = setup,
	.cleanup = cleanup,
	.tcnt = MAX_DEPTH - 1,
};
