// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Fujitsu Ltd.
 *  Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 *  Check that epoll_wait(2) timeouts correctly.
 */

#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>

#include "tst_timer_test.h"

static int epfd, fds[2];
static struct epoll_event epevs[1] = {
       {.events = EPOLLIN}
};

int sample_fn(int clk_id, long long usec)
{
	unsigned int sleep_ms = usec / 1000;

	tst_timer_start(clk_id);
	TEST(epoll_wait(epfd, epevs, 1, sleep_ms));
	tst_timer_stop();
	tst_timer_sample();

	if (TST_RET != 0) {
		tst_res(TFAIL | TTERRNO,
			"epoll_wait() returned %li", TST_RET);
		return 1;
	}

	return 0;
}

static void setup(void)
{
	SAFE_PIPE(fds);

	epfd = epoll_create(1);
	if (epfd == -1)
		tst_brk(TBROK | TERRNO, "epoll_create()");

	epevs[0].data.fd = fds[0];

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fds[0], &epevs[0]))
		tst_brk(TBROK | TERRNO, "epoll_clt(..., EPOLL_CTL_ADD, ...)");
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
	.scall = "epoll_wait()",
	.sample = sample_fn,
	.setup = setup,
	.cleanup = cleanup,
};
