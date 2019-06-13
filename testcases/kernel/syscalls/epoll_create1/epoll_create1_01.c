// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) International Business Machines Corp., 2009
 *
 * Test system call introduced in 2.6.27.
 * http://git.kernel.org/?p=linux/kernel/git/torvalds/linux-2.6.git;a=commit;h=a0998b50c3f0b8fdd265c63e0032f86ebe377dbf
 *
 * This patch adds the new epoll_create1 syscall.  It extends the old
 * epoll_create syscall by one parameter which is meant to hold a flag value.
 * In this patch the only flag support is EPOLL_CLOEXEC which causes the
 * close-on-exec flag for the returned file descriptor to be set. A new name
 * EPOLL_CLOEXEC is introduced which in this implementation must have the same
 * value as O_CLOEXEC.
 */
#include <errno.h>
#include <sys/epoll.h>
#include "tst_test.h"
#include "lapi/epoll.h"
#include "lapi/syscalls.h"

static void verify_epoll_create1(void)
{
	int fd, coe;

	fd = tst_syscall(__NR_epoll_create1, 0);
	if (fd == -1)
		tst_brk(TFAIL | TERRNO, "epoll_create1(0) failed");

	coe = SAFE_FCNTL(fd, F_GETFD);
	if (coe & FD_CLOEXEC)
		tst_brk(TFAIL, "flags=0 set close-on-exec flag");

	SAFE_CLOSE(fd);

	fd = tst_syscall(__NR_epoll_create1, EPOLL_CLOEXEC);
	if (fd == -1)
		tst_brk(TFAIL | TERRNO, "epoll_create1(EPOLL_CLOEXEC) failed");

	coe = SAFE_FCNTL(fd, F_GETFD);
	if ((coe & FD_CLOEXEC) == 0)
		tst_brk(TFAIL, "flags=EPOLL_CLOEXEC didn't set close-on-exec");

	SAFE_CLOSE(fd);

	tst_res(TPASS, "epoll_create1(EPOLL_CLOEXEC) PASSED");
}

static struct tst_test test = {
	.min_kver = "2.6.27",
	.test_all = verify_epoll_create1,
};
