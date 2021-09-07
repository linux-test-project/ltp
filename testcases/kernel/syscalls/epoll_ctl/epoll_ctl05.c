// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2021
 * Author: Xie Ziyao <ziyaoxie@outlook.com>
 */

/*\
 * [Description]
 *
 * Verify that epoll_ctl() fails with ELOOP if fd refers to an epoll instance
 * and this EPOLL_CTL_ADD operation would result in a circular loop of epoll
 * instances monitoring one another.
 */

#include <poll.h>
#include <sys/epoll.h>

#include "tst_test.h"

#define MAX_DEPTH 5

static int epfd, origin_epfd, new_epfd;
static int fd[2];

static struct epoll_event events = {.events = EPOLLIN};

static void setup(void)
{
	int i;

	SAFE_PIPE(fd);

	for (i = 0, epfd = fd[0]; i < MAX_DEPTH; i++, epfd = new_epfd) {
		new_epfd = epoll_create(1);
		if (new_epfd == -1)
			tst_brk(TBROK | TERRNO, "fail to create epoll instance");

		if (i == 0)
			origin_epfd = new_epfd;

		events.data.fd = epfd;
		if (epoll_ctl(new_epfd, EPOLL_CTL_ADD, epfd, &events))
			tst_brk(TBROK | TERRNO, "epoll_clt(..., EPOLL_CTL_ADD, ...)");
	}

	events.data.fd = fd[0];
	if (epoll_ctl(origin_epfd, EPOLL_CTL_DEL, fd[0], &events))
		tst_brk(TBROK | TERRNO, "epoll_clt(..., EPOLL_CTL_DEL, ...)");
}

static void cleanup(void)
{
	if (fd[0])
		SAFE_CLOSE(fd[0]);

	if (fd[1])
		SAFE_CLOSE(fd[1]);
}

static void verify_epoll_ctl(void)
{
	events.data.fd = epfd;
	TST_EXP_FAIL(epoll_ctl(origin_epfd, EPOLL_CTL_ADD, epfd, &events),
		     ELOOP, "epoll_clt(..., EPOLL_CTL_ADD, ...)");
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_epoll_ctl,
};
