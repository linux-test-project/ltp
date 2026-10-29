// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Verify that :manpage:`epoll_ctl(2)` fails with ELOOP when an
 * ``EPOLL_CTL_ADD`` operation would create a minimal two-instance cycle of
 * epoll instances monitoring one another.
 *
 * This is the shortest path through the kernel loop detector. The existing
 * epoll_ctl05 test exercises a long chain closed into a loop.
 *
 * [Algorithm]
 *
 * - create two epoll instances ep_a and ep_b
 * - add ep_b into ep_a with EPOLL_CTL_ADD (succeeds)
 * - add ep_a into ep_b with EPOLL_CTL_ADD, which must fail with ELOOP
 */

#include <sys/epoll.h>

#include "tst_test.h"
#include "tst_epoll.h"

static int ep_a = -1, ep_b = -1;

static void setup(void)
{
	struct epoll_event ev = {.events = EPOLLIN};

	ep_a = SAFE_EPOLL_CREATE1(0);
	ep_b = SAFE_EPOLL_CREATE1(0);

	ev.data.fd = ep_b;
	SAFE_EPOLL_CTL(ep_a, EPOLL_CTL_ADD, ep_b, &ev);
}

static void cleanup(void)
{
	if (ep_a != -1)
		SAFE_CLOSE(ep_a);

	if (ep_b != -1)
		SAFE_CLOSE(ep_b);
}

static void run(void)
{
	struct epoll_event ev = {.events = EPOLLIN, .data.fd = ep_a};

	TST_EXP_FAIL(epoll_ctl(ep_b, EPOLL_CTL_ADD, ep_a, &ev), ELOOP,
		     "epoll_ctl(EPOLL_CTL_ADD) closing a two-instance cycle");
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
};
