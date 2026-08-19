// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test that :manpage:`setns(2)` escapes the failfs root.
 *
 * Entering failfs with :manpage:`fchroot(2)` is hard to undo: a process
 * inside counts as chrooted, so :manpage:`chroot(2)` and :manpage:`fchroot(2)`
 * back out require ``CAP_SYS_CHROOT``. One way out is a pre-opened mount
 * namespace file descriptor: :manpage:`setns(2)` into it resets both the
 * root and the working directory.
 *
 * Root is required because entering failfs requires ``CAP_SYS_CHROOT`` and
 * :manpage:`setns(2)` into the mount namespace requires ``CAP_SYS_ADMIN``.
 *
 * The test runs in a forked child so the root of the parent process is
 * left untouched.
 */

#define _GNU_SOURCE
#include <fcntl.h>
#include <sys/stat.h>
#include "tst_test.h"
#include "lapi/fcntl.h"
#include "lapi/sched.h"
#include "lapi/setns.h"
#include "lapi/syscalls.h"

static void run(void)
{
	if (SAFE_FORK())
		return;

	struct stat realroot, st;
	int nsfd;

	SAFE_STAT("/", &realroot);
	nsfd = SAFE_OPEN("/proc/self/ns/mnt", O_RDONLY);

	TST_EXP_PASS(tst_syscall(__NR_fchroot, FD_FAILFS_ROOT, 0),
		"fchroot() with the FD_FAILFS_ROOT sentinel");

	TST_EXP_FAIL2(open("/", O_PATH), EOPNOTSUPP,
		"absolute lookup after entering failfs");

	TST_EXP_PASS(setns(nsfd, CLONE_NEWNS),
		"setns() back into the mount namespace");

	SAFE_CLOSE(nsfd);

	SAFE_STAT("/", &st);
	TST_EXP_EXPR(st.st_dev == realroot.st_dev &&
		st.st_ino == realroot.st_ino,
		"root restored after setns()");

	SAFE_STAT(".", &st);
	TST_EXP_EXPR(st.st_dev == realroot.st_dev &&
		st.st_ino == realroot.st_ino,
		"working directory restored after setns()");
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.forks_child = 1,
};
