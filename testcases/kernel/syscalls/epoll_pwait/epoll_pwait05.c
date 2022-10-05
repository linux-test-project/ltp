// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * Author: Xie Ziyao <xieziyao@huawei.com>
 */

/*\
 * [Description]
 *
 * Verify that, epoll_pwait2() return -1 and set errno to EINVAL with an
 * invalid timespec.
 */

#include <sys/epoll.h>

#include "tst_test.h"
#include "tst_timer.h"
#include "lapi/epoll.h"

static int efd, sfd[2];
static struct epoll_event e;

static struct test_case_t {
	struct timespec ts;
	int exp_errno;
	const char *desc;
} tc[] = {
	{{.tv_sec = -1}, EINVAL, "ts.tv_sec < 0"},
	{{.tv_nsec = -1}, EINVAL, "ts.tv_nsec < 0"},
	{{.tv_nsec = NSEC_PER_SEC}, EINVAL, "ts.tv_nsec >= NSEC_PER_SEC"},
};

static void run_all(unsigned int n)
{
	TST_EXP_FAIL(epoll_pwait2(efd, &e, 1, &tc[n].ts, NULL),
		     tc[n].exp_errno, "with %s", tc[n].desc);
}

static void setup(void)
{
	SAFE_SOCKETPAIR(AF_UNIX, SOCK_STREAM, 0, sfd);

	efd = epoll_create(1);
	if (efd == -1)
		tst_brk(TBROK | TERRNO, "epoll_create()");

	e.events = EPOLLIN;
	if (epoll_ctl(efd, EPOLL_CTL_ADD, sfd[0], &e))
		tst_brk(TBROK | TERRNO, "epoll_clt(..., EPOLL_CTL_ADD, ...)");
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
	.test = run_all,
	.setup = setup,
	.cleanup = cleanup,
	.tcnt = ARRAY_SIZE(tc),
};
