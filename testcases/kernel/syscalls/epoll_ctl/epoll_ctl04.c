// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2021
 * Author: Xie Ziyao <ziyaoxie@outlook.com>
 */

/*\
 * [Description]
 *
 * Verify that the maximum number of nesting allowed inside epoll sets is 5,
 * otherwise epoll_ctl fails with EINVAL.
 */

#include <poll.h>
#include <sys/epoll.h>

#include "tst_test.h"

#define MAX_DEPTH 5

static int epfd, new_epfd;
static int fd[2];

static struct epoll_event events = {.events = EPOLLIN};

static void setup(void)
{
	int depth;

	SAFE_PIPE(fd);

	for (depth = 0, epfd = fd[0]; depth < MAX_DEPTH; depth++) {
		new_epfd = epoll_create(1);
		if (new_epfd == -1)
			tst_brk(TBROK | TERRNO, "fail to create epoll instance");

		events.data.fd = epfd;
		if (epoll_ctl(new_epfd, EPOLL_CTL_ADD, epfd, &events))
			tst_brk(TBROK | TERRNO, "epoll_clt(..., EPOLL_CTL_ADD, ...)");

		epfd = new_epfd;
	}
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
	new_epfd = epoll_create(1);
	if (new_epfd == -1)
		tst_brk(TBROK | TERRNO, "fail to create epoll instance");

	events.data.fd = epfd;
	TST_EXP_FAIL(epoll_ctl(new_epfd, EPOLL_CTL_ADD, epfd, &events), EINVAL,
		     "epoll_clt(..., EPOLL_CTL_ADD, ...) with number of nesting is 5");
	SAFE_CLOSE(new_epfd);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_epoll_ctl,
};
