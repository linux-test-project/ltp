// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test that :manpage:`execve(2)` is blocked under the failfs root.
 *
 * After :manpage:`fchroot(2)` moved the process root into failfs, loading
 * a binary by absolute path fails with ``EOPNOTSUPP``.
 *
 * Root is required because entering failfs with the ``FD_FAILFS_ROOT``
 * sentinel requires ``CAP_SYS_CHROOT``.
 *
 * The test runs in a forked child so the root of the parent process is
 * left untouched.
 */

#define _GNU_SOURCE
#include <unistd.h>
#include "tst_test.h"
#include "lapi/fcntl.h"
#include "lapi/syscalls.h"

#define CHILD_EXEC "fchroot07_child"

static char child_path[PATH_MAX];

static void run(void)
{
	if (SAFE_FORK())
		return;

	TST_EXP_PASS(tst_syscall(__NR_fchroot, FD_FAILFS_ROOT, 0),
		"fchroot() with the FD_FAILFS_ROOT sentinel");

	if (!SAFE_FORK()) {
		TST_EXP_FAIL(execl(child_path, CHILD_EXEC, NULL), EOPNOTSUPP,
			"absolute exec blocked by the failfs root");
		return;
	}
}

static void setup(void)
{
	if (tst_get_path(CHILD_EXEC, child_path, sizeof(child_path)))
		tst_brk(TCONF, "Couldn't find %s in $PATH", CHILD_EXEC);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_root = 1,
	.forks_child = 1,
	.child_needs_reinit = 1,
};
