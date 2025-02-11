// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * Author: Xie Ziyao <xieziyao@huawei.com>
 */

/*\
 * Check that epoll_pwait and epoll_pwait2 timeouts correctly.
 */

#include <sys/epoll.h>

#include "tst_timer_test.h"
#include "epoll_pwait_var.h"

#define USEC_PER_MSEC (1000L)

static int efd, sfd[2];
static struct epoll_event e;

int sample_fn(int clk_id, long long usec)
{
	unsigned int ms = usec / USEC_PER_MSEC;

	tst_timer_start(clk_id);
	TEST(do_epoll_pwait(efd, &e, 1, ms, NULL));
	tst_timer_stop();
	tst_timer_sample();

	if (TST_RET != 0) {
		tst_res(TFAIL | TTERRNO,
			"do_epoll_pwait() returned %li, expected 0", TST_RET);
		return 1;
	}

	return 0;
}

static void setup(void)
{
	epoll_pwait_init();

	SAFE_SOCKETPAIR(AF_UNIX, SOCK_STREAM, 0, sfd);

	efd = epoll_create(1);
	if (efd == -1)
		tst_brk(TBROK | TERRNO, "epoll_create()");

	e.events = EPOLLIN;
	if (epoll_ctl(efd, EPOLL_CTL_ADD, sfd[0], &e))
		tst_brk(TBROK | TERRNO, "epoll_ctl(..., EPOLL_CTL_ADD, ...)");
}

static void cleanup(void)
{
	if (efd > 0)
		SAFE_CLOSE(efd);

	if (sfd[0] > 0)
		SAFE_CLOSE(sfd[0]);

	if (sfd[1] > 0)
		SAFE_CLOSE(sfd[1]);
}

static struct tst_test test = {
	.scall = "do_epoll_pwait()",
	.sample = sample_fn,
	.setup = setup,
	.cleanup = cleanup,
	.test_variants = TEST_VARIANTS,
};
