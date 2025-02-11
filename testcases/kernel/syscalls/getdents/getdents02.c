// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	         written by Wayne Boyer
 * Copyright (c) 2013 Markos Chandras
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Verify that:
 *
 *   - getdents() fails with EBADF if file descriptor fd is invalid
 *   - getdents() fails with EINVAL if result buffer is too small
 *   - getdents() fails with ENOTDIR if file descriptor does not refer to a directory
 *   - getdents() fails with ENOENT if directory was unlinked()
 *   - getdents() fails with EFAULT if argument points outside the calling process's address space
 */

#define _GNU_SOURCE
#include <errno.h>

#include "tst_test.h"
#include "getdents.h"

#define DIR_MODE	0755
#define MNTPOINT	"mntpoint"
#define TEST_DIR	MNTPOINT "/test_dir"
#define TEST_FILE	MNTPOINT "/test"

static char *dirp;
static size_t size;

static char dirp1_arr[1];
static char *dirp1 = dirp1_arr;
static char *dirp_bad;
static size_t size1 = 1;

static int fd_inv = -5;
static int fd;
static int fd_file;
static int fd_unlinked;

static struct tcase {
	int *fd;
	char **dirp;
	size_t *size;
	int exp_errno;
} tcases[] = {
	{ &fd_inv, &dirp, &size, EBADF },
	{ &fd, &dirp1, &size1, EINVAL },
	{ &fd_file, &dirp, &size, ENOTDIR },
	{ &fd_unlinked, &dirp, &size, ENOENT },
	{ &fd, &dirp_bad, &size, EFAULT },
};

static void setup(void)
{
	getdents_info();

	size = tst_dirp_size();
	dirp = tst_alloc(size);

	fd = SAFE_OPEN(MNTPOINT, O_RDONLY);
	fd_file = SAFE_OPEN(TEST_FILE, O_CREAT | O_RDWR, 0644);

	dirp_bad = tst_get_bad_addr(NULL);

	SAFE_MKDIR(TEST_DIR, DIR_MODE);
	fd_unlinked = SAFE_OPEN(TEST_DIR, O_DIRECTORY);
	SAFE_RMDIR(TEST_DIR);
}

static void run(unsigned int i)
{
	struct tcase *tc = tcases + i;

	TST_EXP_FAIL2(tst_getdents(*tc->fd, *tc->dirp, *tc->size),
		      tc->exp_errno, "fd=%i dirp=%p size=%zu",
		      *tc->fd, *tc->dirp, *tc->size);
}

static struct tst_test test = {
	.test = run,
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test_variants = TEST_VARIANTS,
	.needs_root = 1,
	.all_filesystems = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT
};
