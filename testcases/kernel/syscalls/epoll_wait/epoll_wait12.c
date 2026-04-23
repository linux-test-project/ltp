// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Verify that :manpage:`epoll_wait(2)` reports EPOLLERR when the read end of a
 * pipe is closed while a child process is blocked in epoll_wait on the write
 * end.
 *
 * [Algorithm]
 *
 * - Create a pipe and fill it completely so the write end is not writable.
 * - Register the write end with an epoll instance for EPOLLOUT.
 * - Fork a child that closes its copy of the read end and blocks in
 *   epoll_wait waiting for the pipe to become writable.
 * - Parent waits for the child to enter sleep state, then closes the read
 *   end.
 * - The child's :manpage:`epoll_wait(2)` should return with EPOLLERR set.
 */

#include <sys/epoll.h>
#include <poll.h>

#include "tst_test.h"
#include "tst_epoll.h"

static int efd = -1, fds[2] = {-1, -1};

static void fill_pipe(int fd)
{
	char buf[4096];
	struct pollfd pfd = {.fd = fd, .events = POLLOUT};
	int nfd;

	memset(buf, 'a', sizeof(buf));

	do {
		SAFE_WRITE(SAFE_WRITE_ANY, fd, buf, sizeof(buf));
		nfd = SAFE_POLL(&pfd, 1, 0);
	} while (nfd > 0);
}

static void run(void)
{
	pid_t pid;

	SAFE_PIPE(fds);
	fill_pipe(fds[1]);

	efd = SAFE_EPOLL_CREATE1(0);

	struct epoll_event ev = {
		.events = EPOLLOUT,
		.data.fd = fds[1],
	};

	SAFE_EPOLL_CTL(efd, EPOLL_CTL_ADD, fds[1], &ev);

	pid = SAFE_FORK();
	if (!pid) {
		SAFE_CLOSE(fds[0]);

		TST_EXP_VAL_SILENT(epoll_wait(efd, &ev, 1, 5000), 1);

		if (ev.data.fd != fds[1]) {
			tst_res(TFAIL, "epoll.data.fd %d, expected %d",
				ev.data.fd, fds[1]);
			exit(0);
		}

		if (!(ev.events & EPOLLERR)) {
			tst_res(TFAIL, "events %x, EPOLLERR not set",
				ev.events);
			exit(0);
		}

		tst_res(TPASS,
			"epoll_wait() reported EPOLLERR on broken pipe");
		exit(0);
	}

	TST_PROCESS_STATE_WAIT(pid, 'S', 0);
	SAFE_CLOSE(fds[0]);
	SAFE_CLOSE(fds[1]);
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
