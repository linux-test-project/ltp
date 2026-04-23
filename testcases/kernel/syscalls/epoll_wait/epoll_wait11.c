// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Verify that :manpage:`epoll_wait(2)` reports EPOLLHUP when the write end of
 * a pipe is closed while a child process is blocked in epoll_wait.
 *
 * [Algorithm]
 *
 * - Create a pipe and register the read end with an epoll instance for
 *   EPOLLIN.
 * - Fork a child that blocks in epoll_wait on the pipe read end.
 * - Parent waits for the child to enter sleep state, then closes the write
 *   end.
 * - The child's :manpage:`epoll_wait(2)` should return with EPOLLHUP.
 */

#include <sys/epoll.h>

#include "tst_test.h"
#include "tst_epoll.h"

static int efd = -1, fds[2] = {-1, -1};

static void run(void)
{
	pid_t pid;

	SAFE_PIPE(fds);

	efd = SAFE_EPOLL_CREATE1(0);

	struct epoll_event ev = {
		.events = EPOLLIN,
		.data.fd = fds[0],
	};

	SAFE_EPOLL_CTL(efd, EPOLL_CTL_ADD, fds[0], &ev);

	pid = SAFE_FORK();
	if (!pid) {
		SAFE_CLOSE(fds[1]);

		TEST(epoll_wait(efd, &ev, 1, 5000));
		if (TST_RET != 1) {
			tst_res(TFAIL | TTERRNO,
				"epoll_wait() returned %li, expected 1",
				TST_RET);
			exit(0);
		}

		if (ev.data.fd != fds[0]) {
			tst_res(TFAIL, "epoll.data.fd %d, expected %d",
				ev.data.fd, fds[0]);
			exit(0);
		}

		if (!(ev.events & EPOLLHUP)) {
			tst_res(TFAIL, "events %x, EPOLLHUP not set",
				ev.events);
			exit(0);
		}

		tst_res(TPASS,
			"epoll_wait() reported EPOLLHUP on pipe hangup");
		exit(0);
	}

	TST_PROCESS_STATE_WAIT(pid, 'S', 0);
	SAFE_CLOSE(fds[1]);
	SAFE_CLOSE(fds[0]);
	SAFE_CLOSE(efd);
}

static void cleanup(void)
{
	if (fds[1] != -1)
		SAFE_CLOSE(fds[1]);

	if (fds[0] != -1)
		SAFE_CLOSE(fds[0]);

	if (efd != -1)
		SAFE_CLOSE(efd);
}

static struct tst_test test = {
	.test_all = run,
	.cleanup = cleanup,
	.forks_child = 1,
};
