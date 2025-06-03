// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * Author: Xie Ziyao <xieziyao@huawei.com>
 */

/*\
 * Basic test for epoll_pwait and epoll_pwait2. Checks if data avaiable in a
 * file descriptor are reported correctly in the syscall return value.
 */

#include <sys/epoll.h>

#include "tst_test.h"
#include "epoll_pwait_var.h"

static int efd, sfd[2];
static struct epoll_event e;

static void run(void)
{
	TEST(do_epoll_pwait(efd, &e, 1, NULL, NULL));

	if (TST_RET == 1) {
		tst_res(TPASS, "do_epoll_pwait() succeeded");
		return;
	}
	tst_res(TFAIL, "do_epoll_pwait() returned %li, expected 1", TST_RET);
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
	SAFE_WRITE(SAFE_WRITE_ALL, sfd[1], "w", 1);
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
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.test_variants = TEST_VARIANTS,
};
