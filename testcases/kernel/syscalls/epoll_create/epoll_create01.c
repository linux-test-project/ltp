// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2021
 * Author: Xie Ziyao <ziyaoxie@outlook.com>
 */

/*\
 * [Description]
 *
 * Verify that epoll_create return a nonnegative file descriptor on success.
 *
 * The size argument informed the kernel of the number of file descriptors
 * that the caller expected to add to the epoll instance, but it is no longer
 * required.
 */

#include <sys/epoll.h>

#include "tst_test.h"
#include "lapi/epoll.h"
#include "lapi/syscalls.h"

static int tc[] = {1, INT_MAX};

static void run(unsigned int n)
{
	TST_EXP_FD(tst_syscall(__NR_epoll_create, tc[n]), "epoll_create(%d)", tc[n]);

	if (!TST_PASS)
		return;
	SAFE_CLOSE(TST_RET);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tc),
	.test = run,
};
