// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Fujitsu Ltd.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

/*
 * Test Name: epoll_ctl02.c
 *
 * Description:
 * 1) epoll_ctl(2) fails if epfd is a invalid file descriptor.
 * 2) epoll_ctl(2) fails if fd is a invalid file descriptor.
 * 3) epoll_ctl(2) fails if op is not supported by this interface.
 * 4) epoll_ctl(2) fails if fd is the same as epfd.
 * 5) epoll_ctl(2) fails with EPOLL_CTL_DEL if fd is not registered
 *    with this epoll instance.
 * 6) epoll_ctl(2) fails with EPOLL_CTL_MOD if fd is not registered
 *    with this epoll instance.
 * 7) epoll_ctl(2) fails with EPOLL_CTL_ADD if fd is already registered
 *    with this epoll instance.
 *
 * Expected Result:
 * 1) epoll_ctl(2) should return -1 and set errno to EBADF.
 * 2) epoll_ctl(2) should return -1 and set errno to EBADF.
 * 3) epoll_ctl(2) should return -1 and set errno to EINVAL.
 * 4) epoll_ctl(2) should return -1 and set errno to EINVAL.
 * 5) epoll_ctl(2) should return -1 and set errno to ENOENT.
 * 6) epoll_ctl(2) should return -1 and set errno to ENOENT.
 * 7) epoll_ctl(2) should return -1 and set errno to EEXIST.
 *
 */
#include <sys/epoll.h>
#include <poll.h>
#include <errno.h>
#include "tst_test.h"

static int epfd;
static int fd[2];
static int inv = -1;

static struct epoll_event events[2] = {
	{.events = EPOLLIN},
	{.events = EPOLLOUT},
};

static struct testcase {
	int *epfds;
	int opt;
	int *fds;
	struct epoll_event *ts_event;
	int exp_err;
} tcases[] = {
	{&inv, EPOLL_CTL_ADD, &fd[1], &events[1], EBADF},
	{&epfd, EPOLL_CTL_ADD, &inv, &events[1], EBADF},
	{&epfd, -1, &fd[1], &events[1], EINVAL},
	{&epfd, EPOLL_CTL_ADD, &epfd, &events[1], EINVAL},
	{&epfd, EPOLL_CTL_DEL, &fd[1], &events[1], ENOENT},
	{&epfd, EPOLL_CTL_MOD, &fd[1], &events[1], ENOENT},
	{&epfd, EPOLL_CTL_ADD, &fd[0], &events[0], EEXIST}
};

static void setup(void)
{
	epfd = epoll_create(2);
	if (epfd == -1)
		tst_brk(TBROK | TERRNO, "fail to create epoll instance");

	SAFE_PIPE(fd);

	events[0].data.fd = fd[0];
	events[1].data.fd = fd[1];

	TEST(epoll_ctl(epfd, EPOLL_CTL_ADD, fd[0], &events[0]));
	if (TST_RET == -1)
		tst_brk(TFAIL | TTERRNO, "epoll_ctl() fails to init");
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
	struct testcase *tc = &tcases[n];

	TEST(epoll_ctl(*tc->epfds, tc->opt, *tc->fds,  tc->ts_event));
	if (TST_RET != -1) {
		tst_res(TFAIL, "epoll_ctl() succeeds unexpectedly");
		return;
	}

	if (tc->exp_err == TST_ERR) {
		tst_res(TPASS | TTERRNO, "epoll_ctl() fails as expected");
	} else {
		tst_res(TFAIL | TTERRNO,
			"epoll_ctl() fails unexpectedly, expected %i: %s",
			tc->exp_err, tst_strerrno(tc->exp_err));
	}
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_epoll_ctl,
};
