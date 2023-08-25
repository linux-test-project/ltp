// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that EPOLLONESHOT is correctly handled by epoll_wait.
 * We open a channel, write in it two times and verify that EPOLLIN has been
 * received only once.
 */

#include <poll.h>
#include <sys/epoll.h>
#include "tst_test.h"
#include "tst_epoll.h"

static int fds[2];
static int epfd;

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
	struct epoll_event evt_receive;
	char buff = 'a';

	SAFE_PIPE(fds);

	tst_res(TINFO, "Polling on channel with EPOLLONESHOT");

	epfd = SAFE_EPOLL_CREATE1(0);

	SAFE_EPOLL_CTL(epfd, EPOLL_CTL_ADD, fds[0], &((struct epoll_event) {
		.events = EPOLLIN | EPOLLONESHOT,
		.data.fd = fds[0],
	}));

	tst_res(TINFO, "Write channel for the 1st time. EPOLLIN expected");

	SAFE_WRITE(0, fds[1], &buff, 1);
	TST_EXP_EQ_LI(SAFE_EPOLL_WAIT(epfd, &evt_receive, 10, 0), 1);
	TST_EXP_EQ_LI(evt_receive.events & EPOLLIN, EPOLLIN);
	TST_EXP_EQ_LI(evt_receive.data.fd, fds[0]);

	SAFE_READ(1, fds[0], &buff, 1);
	TST_EXP_EQ_LI(SAFE_EPOLL_WAIT(epfd, &evt_receive, 10, 0), 0);

	tst_res(TINFO, "Write channel for the 2nd time. No events expected");

	SAFE_WRITE(0, fds[1], &buff, 1);
	TST_EXP_EQ_LI(SAFE_EPOLL_WAIT(epfd, &evt_receive, 10, 0), 0);

	SAFE_CLOSE(epfd);
	SAFE_CLOSE(fds[0]);
	SAFE_CLOSE(fds[1]);
}

static struct tst_test test = {
	.cleanup = cleanup,
	.test_all = run,
};
