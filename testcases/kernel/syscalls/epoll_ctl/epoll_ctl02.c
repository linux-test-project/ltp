// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Fujitsu Ltd.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

/*\
 * [Description]
 *
 * Verify that epoll_cnt() fails with:
 *
 * - EBADF if epfd is an invalid fd.
 * - EBADF if fd is an invalid fd.
 * - EINVAL if op is not supported.
 * - EINVAL if fd is the same as epfd.
 * - EINVAL if events is NULL.
 * - ENOENT if fd is not registered with EPOLL_CTL_DEL.
 * - ENOENT if fd is not registered with EPOLL_CTL_MOD.
 * - EEXIST if fd is already registered with EPOLL_CTL_ADD.
 */

#include <poll.h>
#include <sys/epoll.h>

#include "tst_test.h"

static int epfd;
static int fd[2];
static int inv = -1;

static struct epoll_event events[2] = {
	{.events = EPOLLIN},
	{.events = EPOLLOUT},
};

static struct testcase {
	int *epfd;
	int opt;
	int *fd;
	struct epoll_event *event;
	int exp_err;
	const char *desc;
} tc[] = {
	{&inv, EPOLL_CTL_ADD, &fd[1], &events[1], EBADF, "epfd is an invalid fd"},
	{&epfd, EPOLL_CTL_ADD, &inv, &events[1], EBADF, "fd is an invalid fd"},
	{&epfd, -1, &fd[1], &events[1], EINVAL, "op is not supported"},
	{&epfd, EPOLL_CTL_ADD, &epfd, &events[1], EINVAL, "fd is the same as epfd"},
	{&epfd, EPOLL_CTL_ADD, &fd[1], NULL, EFAULT, "events is NULL"},
	{&epfd, EPOLL_CTL_DEL, &fd[1], &events[1], ENOENT, "fd is not registered with EPOLL_CTL_DEL"},
	{&epfd, EPOLL_CTL_MOD, &fd[1], &events[1], ENOENT, "fd is not registered with EPOLL_CTL_MOD"},
	{&epfd, EPOLL_CTL_ADD, &fd[0], &events[0], EEXIST, "fd is already registered with EPOLL_CTL_ADD"}
};

static void setup(void)
{
	epfd = epoll_create(2);
	if (epfd == -1)
		tst_brk(TBROK | TERRNO, "fail to create epoll instance");

	SAFE_PIPE(fd);

	events[0].data.fd = fd[0];
	events[1].data.fd = fd[1];

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd[0], &events[0]))
		tst_brk(TBROK | TERRNO, "epoll_clt(..., EPOLL_CTL_ADD, ...)");
}

static void cleanup(void)
{
	if (epfd)
		SAFE_CLOSE(epfd);

	if (fd[0])
		SAFE_CLOSE(fd[0]);

	if (fd[1])
		SAFE_CLOSE(fd[1]);
}

static void verify_epoll_ctl(unsigned int n)
{
	TST_EXP_FAIL(epoll_ctl(*tc[n].epfd, tc[n].opt, *tc[n].fd, tc[n].event),
		     tc[n].exp_err, "epoll_clt(...) if %s", tc[n].desc);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tc),
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_epoll_ctl,
};
