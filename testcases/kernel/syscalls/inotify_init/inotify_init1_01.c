// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) International Business Machines  Corp., 2009
 * Ported to LTP - Jan 13 2009 - Subrata <subrata@linux.vnet.ibm.com>
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * Verify that inotify_init1() returns a file descriptor and sets
 * the close-on-exec (FD_CLOEXEC) flag on the new file descriptor
 * only when called with IN_CLOEXEC.
 */

#include "tst_test.h"
#include "lapi/syscalls.h"

#define IN_CLOEXEC O_CLOEXEC

static void run(void)
{
	int fd, fd_flags;

	TST_EXP_FD(tst_syscall(__NR_inotify_init1, 0));
	fd = TST_RET;
	fd_flags = SAFE_FCNTL(fd, F_GETFD);
	TST_EXP_EQ_LI(fd_flags & FD_CLOEXEC, 0);
	SAFE_CLOSE(fd);

	TST_EXP_FD(tst_syscall(__NR_inotify_init1, IN_CLOEXEC));
	fd = TST_RET;
	fd_flags = SAFE_FCNTL(fd, F_GETFD);
	TST_EXP_EQ_LI(fd_flags & FD_CLOEXEC, FD_CLOEXEC);
	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = run,
};
