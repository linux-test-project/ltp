// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test path walks under the failfs root.
 *
 * Once :manpage:`fchroot(2)` moved the process root into failfs, only
 * lookups anchored at a file descriptor keep working:
 *
 * - lookups relative to the working directory, which stays in the real
 *   filesystem, keep working
 * - lookups anchored at a pre-opened directory fd keep working, including
 *   resolution of relative symlinks
 * - absolute symlinks restart the walk at the failfs root and fail with
 *   ``EOPNOTSUPP``
 * - ".." walks clamp at the top of the mount tree, not at the failfs
 *   root, so walking up from the working directory lands on the real
 *   root
 *
 * Root is required because entering failfs with the ``FD_FAILFS_ROOT``
 * sentinel requires ``CAP_SYS_CHROOT``.
 *
 * The test runs in a forked child so the root of the parent process is
 * left untouched.
 */

#define _GNU_SOURCE
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include "tst_test.h"
#include "lapi/fcntl.h"
#include "lapi/syscalls.h"

#define RELDIR "rel"
#define ABSDIR "abs"

static char upwards[PATH_MAX];

static void run(void)
{
	if (SAFE_FORK())
		return;

	struct stat realroot, st;
	int dfd, fd;

	SAFE_STAT("/", &realroot);
	dfd = SAFE_OPEN(".", O_RDONLY | O_DIRECTORY);

	TST_EXP_PASS(tst_syscall(__NR_fchroot, FD_FAILFS_ROOT, 0),
		"fchroot() with the FD_FAILFS_ROOT sentinel");

	fd = TST_EXP_FD(openat(AT_FDCWD, ".", O_RDONLY | O_DIRECTORY));
	if (fd != -1)
		SAFE_CLOSE(fd);

	fd = TST_EXP_FD(openat(dfd, "canary", O_WRONLY | O_CREAT, 0600));
	if (fd != -1) {
		SAFE_WRITE(SAFE_WRITE_ALL, fd, "x", 1);
		SAFE_CLOSE(fd);
	}

	fd = TST_EXP_FD(openat(dfd, RELDIR, O_RDONLY));
	if (fd != -1)
		SAFE_CLOSE(fd);

	TST_EXP_FAIL2(openat(dfd, ABSDIR, O_RDONLY), EOPNOTSUPP,
		"resolution of an absolute symlink");

	fd = TST_EXP_FD(openat(AT_FDCWD, upwards, O_PATH));
	if (fd != -1) {
		SAFE_FSTAT(fd, &st);
		SAFE_CLOSE(fd);

		TST_EXP_EXPR(st.st_dev == realroot.st_dev &&
			st.st_ino == realroot.st_ino,
			"'..' walk clamps at the top of the mount tree");
	}

	SAFE_CLOSE(dfd);
}

static void setup(void)
{
	char *tmpdir;
	char abs_path[PATH_MAX];
	struct stat root_st, st;
	int off;

	tmpdir = tst_tmpdir_path();
	snprintf(abs_path, sizeof(abs_path), "%s/%s", tmpdir, "target");

	SAFE_TOUCH("target", 0644, NULL);
	SAFE_SYMLINK("target", RELDIR);
	SAFE_SYMLINK(abs_path, ABSDIR);

	SAFE_STAT("/", &root_st);

	strcpy(upwards, "..");
	off = 2;
	while (1) {
		SAFE_STAT(upwards, &st);

		if (st.st_dev == root_st.st_dev && st.st_ino == root_st.st_ino)
			break;

		if ((size_t)off + sizeof("/..") > sizeof(upwards))
			tst_brk(TBROK, "Path to root too long");

		strcpy(upwards + off, "/..");
		off += 3;
	}
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_root = 1,
	.needs_tmpdir = 1,
	.forks_child = 1,
};
