// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test that the failfs root is inherited across :manpage:`fork(2)`.
 *
 * The root moved into failfs by :manpage:`fchroot(2)` lives in the
 * fs_struct which is duplicated on :manpage:`fork(2)`, so a child of a
 * process with the failfs root also fails every absolute path lookup with
 * ``EOPNOTSUPP``.
 *
 * Root is required because entering failfs with the ``FD_FAILFS_ROOT``
 * sentinel requires ``CAP_SYS_CHROOT``.
 *
 * The test runs in a forked child so the root of the parent process is
 * left untouched.
 */

#define _GNU_SOURCE
#include <fcntl.h>
#include <sys/wait.h>
#include "tst_test.h"
#include "lapi/fcntl.h"
#include "lapi/syscalls.h"

static void run(void)
{
	if (SAFE_FORK())
		return;

	pid_t pid;

	TST_EXP_PASS(tst_syscall(__NR_fchroot, FD_FAILFS_ROOT, 0),
		"fchroot() with the FD_FAILFS_ROOT sentinel");

	pid = SAFE_FORK();
	if (!pid) {
		TST_EXP_FAIL2(open("/", O_PATH), EOPNOTSUPP,
			"absolute lookup in a forked child");
		return;
	}

	SAFE_WAITPID(pid, NULL, 0);
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.forks_child = 1,
};
