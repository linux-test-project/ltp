// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 FUJITSU LIMITED. All Rights Reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that unlink(2) fails with EPERM when target file is marked as
 * immutable or append-only.
 */

#include <sys/ioctl.h>
#include "tst_test.h"
#include "lapi/fs.h"

#define MNTPOINT "mnt"
#define TEST_EPERM_IMMUTABLE MNTPOINT"/test_eperm_immutable"
#define TEST_EPERM_APPEND_ONLY MNTPOINT"/test_eperm_append_only"

static int fd_immutable = -1;
static int fd_append_only = -1;

static struct test_case_t {
	char *filename;
	int *fd;
	int flag;
	char *desc;
} tcases[] = {
	{TEST_EPERM_IMMUTABLE, &fd_immutable, FS_IMMUTABLE_FL,
		"target file is immutable"},
	{TEST_EPERM_APPEND_ONLY, &fd_append_only, FS_APPEND_FL,
		"target file is append-only"},
};

static void setup_inode_flag(const int fd, const int flag, const int set)
{
	int attr;

	SAFE_IOCTL(fd, FS_IOC_GETFLAGS, &attr);

	if (set)
		attr |= flag;
	else
		attr &= ~flag;

	SAFE_IOCTL(fd, FS_IOC_SETFLAGS, &attr);
}

static void setup(void)
{
	int attr;

	/* inode attributes in tmpfs are supported from kernel 6.0
	 * https://lore.kernel.org/all/20220715015912.2560575-1-tytso@mit.edu/
	 */
	if (!strcmp(tst_device->fs_type, "tmpfs") && tst_kvercmp(6, 0, 0) < 0)
		tst_brk(TCONF, "FS_IOC_GETFLAGS on tmpfs not supported for kernel<6.0");

	fd_immutable = SAFE_CREAT(TEST_EPERM_IMMUTABLE, 0600);
	TEST(ioctl(fd_immutable, FS_IOC_GETFLAGS, &attr));

	if (TST_RET == -1 && TST_ERR == ENOTTY) {
		SAFE_CLOSE(fd_immutable);

		tst_brk(TBROK, "Inode attributes not supported by '%s'",
			tst_device->fs_type);
	}

	attr |= FS_IMMUTABLE_FL;
	SAFE_IOCTL(fd_immutable, FS_IOC_SETFLAGS, &attr);

	fd_append_only = SAFE_CREAT(TEST_EPERM_APPEND_ONLY, 0600);
	setup_inode_flag(fd_append_only, FS_APPEND_FL, 1);
}

static void cleanup(void)
{
	if (fd_immutable != -1) {
		setup_inode_flag(fd_immutable, FS_IMMUTABLE_FL, 0);
		SAFE_CLOSE(fd_immutable);
	}

	if (fd_append_only != -1) {
		setup_inode_flag(fd_append_only, FS_APPEND_FL, 0);
		SAFE_CLOSE(fd_append_only);
	}
}

static void verify_unlink(unsigned int i)
{
	struct test_case_t *tc = &tcases[i];

	TST_EXP_FAIL(unlink(tc->filename), EPERM, "%s", tc->desc);

	/* If unlink() succeeded unexpectedly, test file should be restored. */
	if (!TST_RET) {
		*(tc->fd) = SAFE_CREAT(tc->filename, 0600);
		setup_inode_flag(*(tc->fd), tc->flag, 1);
	}
}

static struct tst_test test = {
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.cleanup = cleanup,
	.test = verify_unlink,
	.mntpoint = MNTPOINT,
	.needs_root = 1,
	.format_device = 1,
	.mount_device = 1,
	.all_filesystems = 1,
	.skip_filesystems = (const char *const[]) {
		"fuse",
		"exfat",
		"vfat",
		"ntfs",
		NULL
	},
};
