// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test the :manpage:`fchroot(2)` error paths for invalid arguments:
 *
 * - a non-zero flags argument fails with ``EINVAL``
 * - an invalid fd fails with ``EBADF``, including the ``FD_PIDFS_ROOT`` and
 *   ``FD_NSFS_ROOT`` sentinels which :manpage:`fchroot(2)` does not accept
 * - a fd referring to a regular file fails with ``ENOTDIR``
 *
 * All these checks happen before the ``CAP_SYS_CHROOT`` check, so the test
 * needs no privileges.
 */

#include <fcntl.h>
#include "tst_test.h"
#include "lapi/fcntl.h"
#include "lapi/syscalls.h"

#define FILENAME "file.txt"

static int dir_fd = -1;
static int file_fd = -1;
static int bad_fd = -1;
static int failfs_root = FD_FAILFS_ROOT;
static int pidfs_root = FD_PIDFS_ROOT;
static int nsfs_root = FD_NSFS_ROOT;

static struct tcase {
	int *fd;
	unsigned int flags;
	int exp_errno;
	const char *desc;
} tcases[] = {
	{&dir_fd, 1, EINVAL, "non-zero flags with a directory fd"},
	{&failfs_root, 1, EINVAL, "non-zero flags with FD_FAILFS_ROOT"},
	{&bad_fd, 0, EBADF, "invalid fd"},
	{&pidfs_root, 0, EBADF, "FD_PIDFS_ROOT sentinel"},
	{&nsfs_root, 0, EBADF, "FD_NSFS_ROOT sentinel"},
	{&file_fd, 0, ENOTDIR, "fd referring to a regular file"},
};

static void run(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	TST_EXP_FAIL(tst_syscall(__NR_fchroot, *tc->fd, tc->flags),
		tc->exp_errno, "fchroot() with %s", tc->desc);
}

static void setup(void)
{
	dir_fd = SAFE_OPEN(".", O_PATH | O_DIRECTORY);
	file_fd = SAFE_OPEN(FILENAME, O_CREAT | O_EXCL | O_WRONLY, 0644);
}

static void cleanup(void)
{
	if (dir_fd != -1)
		SAFE_CLOSE(dir_fd);

	if (file_fd != -1)
		SAFE_CLOSE(file_fd);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
