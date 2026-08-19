// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test :manpage:`fchroot(2)` with the ``FD_FAILFS_ROOT`` sentinel as a
 * privileged process.
 *
 * :manpage:`fchroot(2)` was introduced in Linux v7.3 together with failfs,
 * a kernel internal filesystem where every operation fails with
 * ``EOPNOTSUPP``. The ``FD_FAILFS_ROOT`` sentinel moves the process root
 * there without needing a file descriptor: it is the ``fs_struct`` equivalent
 * of ``RESOLVE_BENEATH``.
 *
 * The test verifies that entering failfs succeeds with ``CAP_SYS_CHROOT`` and
 * that every absolute path lookup then fails with ``EOPNOTSUPP``. The working
 * directory, left behind in the real filesystem, is consequently reported
 * as unreachable by :manpage:`getcwd(2)`.
 *
 * The test runs in a forked child so the root of the parent process is
 * left untouched.
 */

#define _GNU_SOURCE
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include "tst_test.h"
#include "lapi/fcntl.h"
#include "lapi/syscalls.h"

#define UNR_TAG "(unreachable)/"

static void run(void)
{
	if (SAFE_FORK())
		return;

	char buf[PATH_MAX];

	TST_EXP_PASS(tst_syscall(__NR_fchroot, FD_FAILFS_ROOT, 0),
		"fchroot() with the FD_FAILFS_ROOT sentinel");
	if (!TST_PASS)
		return;

	TST_EXP_FAIL2(open("/dev/zero", O_RDONLY), EOPNOTSUPP,
		"absolute file open");

	TST_EXP_FAIL(mkdir("/foo", 0700), EOPNOTSUPP,
		"absolute directory creation");

	/*
	 * The libc getcwd() wrapper rejects the "(unreachable)"
	 * prefix produced by the kernel, so call the raw syscall.
	 */
	TEST(tst_syscall(__NR_getcwd, buf, sizeof(buf)));
	if (TST_RET > 0)
		TST_EXP_EQ_STRN(buf, UNR_TAG, sizeof(UNR_TAG) - 1);
	else
		tst_res(TFAIL | TTERRNO, "getcwd() failed");
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.needs_tmpdir = 1,
	.forks_child = 1,
};
