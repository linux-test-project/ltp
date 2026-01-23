// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Wei Gao <wegao@suse.com>
 */

#include "tst_test.h"
#include "tst_safe_stdio.h"

#define INODE_SIZE 128
#define BLOCK_SIZE 1024
#define MKFS_SIZE_VAL 10240

#define MOUNT_POINT "mount_test_filesystems"

static int check_inode_size(unsigned int size)
{
	char path[PATH_MAX];
	char line[PATH_MAX];
	FILE *tune2fs;
	char str_size[NAME_MAX];

	snprintf(str_size, sizeof(str_size), "%u", size);
	snprintf(path, sizeof(path), "tune2fs -l %s 2>&1", tst_device->dev);
	tune2fs = SAFE_POPEN(path, "r");

	while (fgets(line, PATH_MAX, tune2fs) != NULL) {
		if (strstr(line, "Inode size:") && strstr(line, str_size))
			return 0;
	}

	pclose(tune2fs);
	return -1;
}

static int check_mnt_data(char *opt)
{
	FILE *fp;
	char line[PATH_MAX];

	fp = SAFE_FOPEN("/proc/mounts", "r");

	while (fgets(line, PATH_MAX, fp) != NULL) {
		if (strstr(line, tst_device->dev) && strstr(line, opt))
			return 0;
	}
	SAFE_FCLOSE(fp);
	return -1;
}

static int check_mkfs_size_opt(unsigned int size)
{
	char path[PATH_MAX];
	char line[PATH_MAX];
	FILE *dumpe2fs;
	char str_size[NAME_MAX];

	snprintf(str_size, sizeof(str_size), "%u", size);
	snprintf(path, sizeof(path), "dumpe2fs -h %s 2>&1", tst_device->dev);
	dumpe2fs = SAFE_POPEN(path, "r");

	while (fgets(line, PATH_MAX, dumpe2fs) != NULL) {
		if (strstr(line, "Block count:") && strstr(line, str_size))
			return 0;
	}

	pclose(dumpe2fs);
	return -1;
}

static void do_test(void)
{
	long fs_type;

	fs_type = tst_fs_type(MOUNT_POINT);

	if (fs_type == TST_EXT234_MAGIC) {
		TST_EXP_PASS((check_inode_size(INODE_SIZE)));
		TST_EXP_PASS((check_mkfs_size_opt(MKFS_SIZE_VAL)));
	}

	if (fs_type == TST_XFS_MAGIC)
		TST_EXP_PASS((check_mnt_data("usrquota")));
}

static struct tst_test test = {
	.test_all = do_test,
	.needs_root = 1,
	.mntpoint = MOUNT_POINT,
	.mount_device = 1,
	.needs_cmds = (struct tst_cmd[]) {
		{.cmd = "tune2fs"},
		{.cmd = "dumpe2fs"},
		{}
	},
	.filesystems = (struct tst_fs []) {
		{
			.type = "ext3",
			.mkfs_opts = (const char *const []){"-I", TST_TO_STR(INODE_SIZE), "-b", TST_TO_STR(BLOCK_SIZE), NULL},
			.mkfs_size_opt = TST_TO_STR(MKFS_SIZE_VAL),
		},
		{
			.type = "xfs",
			.mnt_data = "usrquota",
		},
		{}
	},

};
