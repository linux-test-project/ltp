// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2021
 * Author: Xie Ziyao <ziyaoxie@outlook.com>
 */

/*\
 * [Description]
 *
 * Check that epoll_ctl returns zero with different combinations of events on
 * success.
 */

#include <poll.h>
#include <sys/epoll.h>

#include "tst_test.h"
#include "tst_bitmap.h"

#define NUM_EPOLL_EVENTS 8
#define WIDTH_EPOLL_EVENTS 256

static int epfd, fds[2];
static struct epoll_event events = {.events = EPOLLIN};

static unsigned int events_type[NUM_EPOLL_EVENTS] = {
		EPOLLIN, EPOLLOUT, EPOLLPRI, EPOLLERR,
		EPOLLHUP, EPOLLET, EPOLLONESHOT, EPOLLRDHUP
};

static void run_all(void)
{
	unsigned int i, j, events_bitmap;

	for (i = 0; i < WIDTH_EPOLL_EVENTS; i++) {
		events_bitmap = 0;

		for (j = 0; j < NUM_EPOLL_EVENTS; j++)
			events_bitmap |= (events_type[j] * TST_IS_BIT_SET(i, j));

		events.events = events_bitmap;
		TST_EXP_PASS(epoll_ctl(epfd, EPOLL_CTL_MOD, fds[0], &events),
			     "epoll_ctl(..., EPOLL_CTL_MOD, ...) with events.events=%08x",
			     events.events);
	}
}

static void setup(void)
{
	epfd = epoll_create(1);
	if (epfd == -1)
		tst_brk(TBROK | TERRNO, "fail to create epoll instance");

	SAFE_PIPE(fds);
	events.data.fd = fds[0];

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fds[0], &events))
		tst_brk(TBROK | TERRNO, "epoll_ctl(..., EPOLL_CTL_ADD, ...)");
}

static void cleanup(void)
{
	if (epfd)
		SAFE_CLOSE(epfd);

	if (fds[0])
		SAFE_CLOSE(fds[0]);

	if (fds[1])
		SAFE_CLOSE(fds[1]);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run_all,
};
