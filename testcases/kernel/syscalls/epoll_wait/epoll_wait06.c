// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that edge triggering is correctly handled by epoll, for both EPOLLIN
 * and EPOLLOUT.
 *
 * [Algorithm]
 *
 * - The file descriptors for non-blocking pipe are registered on an epoll
 *   instance.
 * - A pipe writer writes data on the write side of the pipe.
 * - A call to epoll_wait() is done that will return a EPOLLIN event.
 * - The pipe reader reads half of data from rfd.
 * - A call to epoll_wait() should hang because there's data left to read.
 * - The pipe reader reads remaining data from rfd.
 * - A call to epoll_wait() should return a EPOLLOUT event.
 */

#define _GNU_SOURCE

#include <fcntl.h>
#include "tst_test.h"
#include "tst_epoll.h"

static size_t write_size;
static size_t read_size;
static int fds[2];
static int epfd;

static void setup(void)
{
	write_size = getpagesize();
	read_size = write_size / 2;

	SAFE_PIPE2(fds, O_NONBLOCK);

	/* EPOLLOUT will be raised when buffer became empty after becoming full */
	SAFE_FCNTL(fds[1], F_SETPIPE_SZ, write_size);
}

static void cleanup(void)
{
	if (epfd > 0)
		SAFE_CLOSE(epfd);

	if (fds[0] > 0)
		SAFE_CLOSE(fds[0]);

	if (fds[1] > 0)
		SAFE_CLOSE(fds[1]);
}

static void run(void)
{
	char buff[write_size];
	struct epoll_event evt_receive;

	tst_res(TINFO, "Polling on channel with EPOLLET");

	epfd = SAFE_EPOLL_CREATE1(0);

	SAFE_EPOLL_CTL(epfd, EPOLL_CTL_ADD, fds[0], &((struct epoll_event) {
		.events = EPOLLIN | EPOLLET,
		.data.fd = fds[0],
	}));
	SAFE_EPOLL_CTL(epfd, EPOLL_CTL_ADD, fds[1], &((struct epoll_event) {
		.events = EPOLLOUT | EPOLLET,
		.data.fd = fds[1],
	}));

	tst_res(TINFO, "Write bytes on channel: %zu bytes", write_size);

	memset(buff, 'a', write_size);
	SAFE_WRITE(SAFE_WRITE_ANY, fds[1], buff, write_size);
	TST_EXP_FAIL(write(fds[1], buff, write_size), EAGAIN, "write() failed");

	TST_EXP_EQ_LI(SAFE_EPOLL_WAIT(epfd, &evt_receive, 1, 0), 1);
	TST_EXP_EQ_LI(evt_receive.data.fd, fds[0]);
	TST_EXP_EQ_LI(evt_receive.events & EPOLLIN, EPOLLIN);

	tst_res(TINFO, "Read half bytes from channel: %zu bytes", read_size);

	memset(buff, 0, write_size);
	SAFE_READ(1, fds[0], buff, read_size);

	TST_EXP_EQ_LI(SAFE_EPOLL_WAIT(epfd, &evt_receive, 1, 0), 0);

	tst_res(TINFO, "Read remaining bytes from channel: %zu bytes", read_size);

	SAFE_READ(1, fds[0], buff + read_size, read_size);
	TST_EXP_FAIL(read(fds[0], buff, read_size), EAGAIN, "read() failed");

	TST_EXP_EQ_LI(SAFE_EPOLL_WAIT(epfd, &evt_receive, 1, 0), 1);
	TST_EXP_EQ_LI(evt_receive.data.fd, fds[1]);
	TST_EXP_EQ_LI(evt_receive.events & EPOLLOUT, EPOLLOUT);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
};
