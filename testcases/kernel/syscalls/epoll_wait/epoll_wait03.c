// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Fujitsu Ltd.
 * Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 * Copyright (c) 2021 Xie Ziyao <xieziyao@huawei.com>
 */

/*\
 * [Description]
 *
 * Basic test for epoll_wait:
 *
 * - epoll_wait fails with EBADF if epfd is not a valid file descriptor.
 * - epoll_wait fails with EINVAL if epfd is not an epoll file descriptor.
 * - epoll_wait fails with EINVAL if maxevents is less than zero.
 * - epoll_wait fails with EINVAL if maxevents is equal to zero.
 * - epoll_wait fails with EFAULT if the memory area pointed to by events is
 *   not accessible with write permissions.
 */

#include <sys/mman.h>
#include <sys/epoll.h>

#include "tst_test.h"

static struct epoll_event epevs[1] = {
	{.events = EPOLLOUT}
};

static struct epoll_event *ev_rdonly, *ev_rdwr = epevs;
static int fds[2], epfd, inv_epfd, bad_epfd = -1;

static struct test_case_t {
	int *epfd;
	struct epoll_event **ev;
	int maxevents;
	int exp_errno;
	const char *desc;
} tc[] = {
	{&bad_epfd, &ev_rdwr, 1, EBADF, "epfd is not a valid fd"},
	{&inv_epfd, &ev_rdwr, 1, EINVAL, "epfd is not an epoll fd"},
	{&epfd, &ev_rdwr, -1, EINVAL, "maxevents is less than zero"},
	{&epfd, &ev_rdwr, 0, EINVAL, "maxevents is equal to zero"},
	{&epfd, &ev_rdonly, 1, EFAULT, "events has no write permissions"}
};

static void setup(void)
{
	ev_rdonly = SAFE_MMAP(NULL, getpagesize(), PROT_READ,
			      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	SAFE_PIPE(fds);

	epfd = epoll_create(1);
	if (epfd == -1)
		tst_brk(TBROK | TERRNO, "epoll_create()");

	epevs[0].data.fd = fds[1];

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fds[1], &epevs[0]))
		tst_brk(TBROK | TERRNO, "epoll_ctl(..., EPOLL_CTL_ADD, ...)");
}

static void verify_epoll_wait(unsigned int n)
{
	TST_EXP_FAIL(epoll_wait(*tc[n].epfd, *tc[n].ev, tc[n].maxevents, -1),
		     tc[n].exp_errno, "epoll_wait() %s", tc[n].desc);
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

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_epoll_wait,
	.tcnt = ARRAY_SIZE(tc),
};
