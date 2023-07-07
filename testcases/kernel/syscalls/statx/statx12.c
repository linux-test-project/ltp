// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*\
 * [Description]
 *
 * It is a basic test for STATX_ATTR_MOUNT_ROOT flag.
 *
 * This flag indicates whether the path or fd refers to the root of a mount
 * or not.
 *
 * Minimum Linux version required is v5.8.
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "tst_test.h"
#include "lapi/stat.h"

#define MNTPOINT "mntpoint"
#define TESTFILE MNTPOINT"/testfile"

static int dir_fd = -1, file_fd = -1;

static struct tcase {
	const char *path;
	bool mnt_root;
	int *fd;
} tcases[] = {
	{MNTPOINT, 1, &dir_fd},
	{TESTFILE, 0, &file_fd}
};

static void verify_statx(unsigned int n)
{
	struct tcase *tc = &tcases[n/2];
	struct statx buf;
	bool flag = n % 2;

	if (flag) {
		tst_res(TINFO, "Testing %s with STATX_ATTR_MOUNT_ROOT by fd",
				tc->path);
		TST_EXP_PASS_SILENT(statx(*tc->fd, "", AT_EMPTY_PATH, 0, &buf));
	} else {
		tst_res(TINFO, "Testing %s with STATX_ATTR_MOUNT_ROOT by path",
				tc->path);
		TST_EXP_PASS_SILENT(statx(AT_FDCWD, tc->path, 0, 0, &buf));
	}

	if (!(buf.stx_attributes_mask & STATX_ATTR_MOUNT_ROOT)) {
		tst_res(TCONF, "Filesystem does not support STATX_ATTR_MOUNT_ROOT");
		return;
	}

	if (buf.stx_attributes & STATX_ATTR_MOUNT_ROOT) {
		tst_res(tc->mnt_root ? TPASS : TFAIL,
			"STATX_ATTR_MOUNT_ROOT flag is set");
	} else {
		tst_res(tc->mnt_root ? TFAIL : TPASS,
			"STATX_ATTR_MOUNT_ROOT flag is not set");
	}
}

static void setup(void)
{
	SAFE_CREAT(TESTFILE, 0755);
	dir_fd = SAFE_OPEN(MNTPOINT, O_DIRECTORY);
	file_fd = SAFE_OPEN(TESTFILE, O_RDWR);
}

static void cleanup(void)
{
	if (dir_fd > -1)
		SAFE_CLOSE(dir_fd);

	if (file_fd > -1)
		SAFE_CLOSE(file_fd);
}

static struct tst_test test = {
	.test = verify_statx,
	.setup = setup,
	.cleanup = cleanup,
	.mntpoint = MNTPOINT,
	.mount_device = 1,
	.all_filesystems = 1,
	.needs_root = 1,
	.tcnt = 2 * ARRAY_SIZE(tcases)
};
