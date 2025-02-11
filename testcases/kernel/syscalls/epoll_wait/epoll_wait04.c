// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * Author: Xie Ziyao <xieziyao@huawei.com>
 */

/*\
 * Check that a timeout equal to zero causes epoll_wait() to return immediately.
 */

#include <sys/epoll.h>

#include "tst_test.h"
#include "tst_timer_test.h"

#define USEC_PRECISION 1000	/* Error margin allowed in usec */

static int epfd, fds[2];
static struct epoll_event epevs[1] = {
	{.events = EPOLLIN}
};

static void run(void)
{
	tst_timer_start(CLOCK_MONOTONIC);
	TEST(epoll_wait(epfd, epevs, 1, 0));
	tst_timer_stop();

	if (TST_RET != 0)
		tst_res(TFAIL | TTERRNO, "epoll_wait() returned %li", TST_RET);

	if (tst_timer_elapsed_us() <= USEC_PRECISION)
		tst_res(TPASS, "epoll_wait() returns immediately with a timeout equal to zero");
	else
		tst_res(TFAIL, "epoll_wait() waited for %llius with a timeout equal to zero",
			tst_timer_elapsed_us());
}

static void setup(void)
{
	SAFE_PIPE(fds);

	epfd = epoll_create(1);
	if (epfd == -1)
		tst_brk(TBROK | TERRNO, "epoll_create()");

	epevs[0].data.fd = fds[0];

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fds[0], &epevs[0]))
		tst_brk(TBROK | TERRNO, "epoll_ctl(..., EPOLL_CTL_ADD, ...)");
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
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
};
