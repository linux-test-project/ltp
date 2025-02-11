// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 */

/*\
 * Verify that readdir will fail with:
 *
 * - ENOENT when passed a fd to a deleted directory
 * - ENOTDIR when passed fd that does not point to a directory
 * - EBADFD when passed an invalid fd
 * - EFAULT when passed invalid buffer pointer
 */

#include <sys/stat.h>
#include "tst_test.h"
#include "lapi/syscalls.h"
#include "lapi/readdir.h"

#define MNTPOINT	"mntpoint"
#define TEST_DIR	MNTPOINT "/test_dir"
#define TEST_DIR4	MNTPOINT "/test_dir4"
#define TEST_FILE	MNTPOINT "/test_file"
#define DIR_MODE	0755

static unsigned int del_dir_fd, file_fd;
static unsigned int invalid_fd = 999;
static unsigned int dir_fd;
static struct old_linux_dirent dirp;

static struct tcase {
	unsigned int *fd;
	struct old_linux_dirent *dirp;
	unsigned int count;
	int exp_errno;
	char *desc;
} tcases[] = {
	{&del_dir_fd, &dirp, sizeof(struct old_linux_dirent), ENOENT, "directory deleted"},
	{&file_fd, &dirp, sizeof(struct old_linux_dirent), ENOTDIR, "not a directory"},
	{&invalid_fd, &dirp, sizeof(struct old_linux_dirent), EBADF, "invalid fd"},
	{&dir_fd, (struct old_linux_dirent *)-1,
	 sizeof(struct old_linux_dirent), EFAULT, "invalid buffer pointer"},
};

static void setup(void)
{
	unsigned int i;

	SAFE_MKDIR(TEST_DIR, DIR_MODE);
	del_dir_fd = SAFE_OPEN(TEST_DIR, O_RDONLY | O_DIRECTORY);
	SAFE_RMDIR(TEST_DIR);

	file_fd = SAFE_OPEN(TEST_FILE, O_RDWR | O_CREAT, 0777);

	SAFE_MKDIR(TEST_DIR4, DIR_MODE);
	dir_fd = SAFE_OPEN(TEST_DIR4, O_RDONLY | O_DIRECTORY);

	for (i = 0; i < ARRAY_SIZE(tcases); i++) {
		if (tcases[i].exp_errno == EFAULT)
			tcases[i].dirp = tst_get_bad_addr(NULL);
	}
}

static void verify_readdir(unsigned int nr)
{
	struct tcase *tc = &tcases[nr];

	TST_EXP_FAIL(tst_syscall(__NR_readdir, *tc->fd, tc->dirp, tc->count),
			tc->exp_errno, "readdir() with %s", tc->desc);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.test = verify_readdir,
	.needs_root = 1,
	.all_filesystems = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT
};
