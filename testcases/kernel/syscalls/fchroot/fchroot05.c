// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test that the failfs root can not be referenced once it is the process
 * root.
 *
 * After :manpage:`fchroot(2)` moved the root into failfs, the root
 * directory can not be opened anymore, not even with ``O_PATH``, because the
 * walk lands on the failfs root as its terminal. The root also can not
 * be pinned by following the /proc/self/root magic link into it, although
 * :manpage:`readlink(2)` still names it as "failfs:/" since it does not
 * follow the link.
 *
 * Root is required because entering failfs with the ``FD_FAILFS_ROOT``
 * sentinel requires ``CAP_SYS_CHROOT``.
 *
 * /proc must be opened before entering failfs because every absolute path
 * lookup fails once the root is unreachable. The test runs in a forked
 * child so the root of the parent process is left untouched.
 */

#define _GNU_SOURCE
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include "tst_test.h"
#include "lapi/fcntl.h"
#include "lapi/syscalls.h"

static void run(void)
{
	if (SAFE_FORK())
		return;

	char buf[PATH_MAX];
	struct stat st;
	int procfd;

	procfd = SAFE_OPEN("/proc", O_PATH | O_DIRECTORY);

	TST_EXP_PASS(tst_syscall(__NR_fchroot, FD_FAILFS_ROOT, 0),
		"fchroot() with the FD_FAILFS_ROOT sentinel");

	TST_EXP_FAIL2(open("/", O_RDONLY | O_DIRECTORY), EOPNOTSUPP,
		"open() of the failfs root");

	TST_EXP_FAIL2(open("/", O_PATH), EOPNOTSUPP,
		"O_PATH open() of the failfs root");

	TST_EXP_FAIL2(openat(procfd, "self/root", O_PATH), EOPNOTSUPP,
		"pin of the root via /proc/self/root");

	TST_EXP_FAIL(fstatat(procfd, "self/root", &st, 0), EOPNOTSUPP,
		"stat of the root via /proc/self/root");

	TEST(readlinkat(procfd, "self/root", buf, sizeof(buf) - 1));
	if (TST_RET < 0) {
		tst_res(TFAIL | TTERRNO, "readlinkat() of /proc/self/root");
	} else {
		buf[TST_RET] = '\0';
		TST_EXP_EQ_STR(buf, "failfs:/");
	}
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.forks_child = 1,
};
