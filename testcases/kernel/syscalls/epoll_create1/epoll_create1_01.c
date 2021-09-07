// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) International Business Machines Corp., 2009
 */

/*\
 * [Description]
 *
 * Verify that epoll_create1 sets the close-on-exec flag for the returned
 * file descriptor with EPOLL_CLOEXEC.
 */

#include <sys/epoll.h>

#include "tst_test.h"
#include "lapi/epoll.h"
#include "lapi/syscalls.h"

static struct test_case_t {
	int flags;
	int exp_flag;
	const char *desc;
} tc[] = {
	{0, 0, "without EPOLL_CLOEXEC"},
	{EPOLL_CLOEXEC, 1, "with EPOLL_CLOEXEC"}
};

static void run(unsigned int n)
{
	int fd, coe;

	fd = tst_syscall(__NR_epoll_create1, tc[n].flags);
	if (fd == -1)
		tst_brk(TFAIL | TERRNO, "epoll_create1(...) failed");

	coe = SAFE_FCNTL(fd, F_GETFD);
	if ((coe & FD_CLOEXEC) != tc[n].exp_flag)
		tst_res(TFAIL, "epoll_create1(...) %s", tc[n].desc);
	else
		tst_res(TPASS, "epoll_create1(...) %s", tc[n].desc);

	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.min_kver = "2.6.27",
	.tcnt = ARRAY_SIZE(tc),
	.test = run,
};
