// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) International Business Machines  Corp., 2009
 * Ported to LTP - Jan 13 2009 - Subrata <subrata@linux.vnet.ibm.com>
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * Verify that inotify_init1() returns a file descriptor and sets the
 * O_NONBLOCK file status flag on the open file description referred
 * to by the new file descriptor only when called with IN_NONBLOCK.
 */

#include "tst_test.h"
#include "lapi/syscalls.h"

#define IN_NONBLOCK O_NONBLOCK

static void run(void)
{
	int fd, flags;

	TST_EXP_FD(tst_syscall(__NR_inotify_init1, 0));
	fd = TST_RET;
	flags = SAFE_FCNTL(fd, F_GETFL);
	TST_EXP_EQ_LI(flags & O_NONBLOCK, 0);
	SAFE_CLOSE(fd);

	TST_EXP_FD(tst_syscall(__NR_inotify_init1, IN_NONBLOCK));
	fd = TST_RET;
	flags = SAFE_FCNTL(fd, F_GETFL);
	TST_EXP_EQ_LI(flags & O_NONBLOCK, O_NONBLOCK);
	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = run,
};
