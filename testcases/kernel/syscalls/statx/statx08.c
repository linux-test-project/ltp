// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*\
 * [Description]
 *
 * This case tests whether the attributes field of statx received expected value
 * by using flags in the stx_attributes_mask field of statx.
 * File set with following flags by using SAFE_IOCTL:
 *
 * - STATX_ATTR_COMPRESSED: The file is compressed by the filesystem.
 * - STATX_ATTR_IMMUTABLE: The file cannot be modified.
 * - STATX_ATTR_APPEND: The file can only be opened in append mode for writing.
 * - STATX_ATTR_NODUMP: File is not a candidate for backup when a backup
 *                        program such as dump(8) is run.
 *
 * Two directories are tested.
 * First directory has all flags set. Second directory has no flags set.
 *
 * Minimum kernel version required is 4.11.
 */

#define _GNU_SOURCE
#include "tst_test.h"
#include "lapi/fs.h"
#include <stdlib.h>
#include "lapi/stat.h"

#define MOUNT_POINT "mntpoint"
#define TESTDIR_FLAGGED MOUNT_POINT"/test_dir1"
#define TESTDIR_UNFLAGGED MOUNT_POINT"/test_dir2"

static int fd, clear_flags;
static int supp_compr = 1, supp_append = 1, supp_immutable = 1, supp_nodump = 1;

static void run(unsigned int flag)
{
	struct statx buf;

	TEST(statx(AT_FDCWD, flag ? TESTDIR_FLAGGED : TESTDIR_UNFLAGGED, 0, 0, &buf));
	if (TST_RET == 0)
		tst_res(TPASS,
			"sys_statx(AT_FDCWD, %s, 0, 0, &buf)",
			flag ? TESTDIR_FLAGGED : TESTDIR_UNFLAGGED);
	else
		tst_brk(TFAIL | TTERRNO,
			"sys_statx(AT_FDCWD, %s, 0, 0, &buf)",
			flag ? TESTDIR_FLAGGED : TESTDIR_UNFLAGGED);

	if (supp_compr) {
		if (buf.stx_attributes & STATX_ATTR_COMPRESSED)
			tst_res(flag ? TPASS : TFAIL,
				"STATX_ATTR_COMPRESSED flag is set");
		else
			tst_res(flag ? TFAIL : TPASS,
				"STATX_ATTR_COMPRESSED flag is not set");
	}
	if (supp_append) {
		if (buf.stx_attributes & STATX_ATTR_APPEND)
			tst_res(flag ? TPASS : TFAIL,
				"STATX_ATTR_APPEND flag is set");
		else
			tst_res(flag ? TFAIL : TPASS,
				"STATX_ATTR_APPEND flag is not set");
	}
	if (supp_immutable) {
		if (buf.stx_attributes & STATX_ATTR_IMMUTABLE)
			tst_res(flag ? TPASS : TFAIL,
				"STATX_ATTR_IMMUTABLE flag is set");
		else
			tst_res(flag ? TFAIL : TPASS,
				"STATX_ATTR_IMMUTABLE flag is not set");
	}
	if (supp_nodump) {
		if (buf.stx_attributes & STATX_ATTR_NODUMP)
			tst_res(flag ? TPASS : TFAIL,
				"STATX_ATTR_NODUMP flag is set");
		else
			tst_res(flag ? TFAIL : TPASS,
				"STATX_ATTR_NODUMP flag is not set");
	}
}

static void caid_flags_setup(void)
{
	int attr, ret;

	fd = SAFE_OPEN(TESTDIR_FLAGGED, O_RDONLY | O_DIRECTORY);

	ret = ioctl(fd, FS_IOC_GETFLAGS, &attr);
	if (ret < 0) {
		if (errno == ENOTTY)
			tst_brk(TCONF | TERRNO, "FS_IOC_GETFLAGS not supported");

		/* ntfs3g fuse fs returns wrong errno for unimplemented ioctls */
		if (!strcmp(tst_device->fs_type, "ntfs")) {
			tst_brk(TCONF | TERRNO,
				"ntfs3g does not support FS_IOC_GETFLAGS");
		}

		tst_brk(TBROK | TERRNO, "ioctl(%i, FS_IOC_GETFLAGS, ...)", fd);
	}

	if (supp_compr)
		attr |= FS_COMPR_FL;
	if (supp_append)
		attr |= FS_APPEND_FL;
	if (supp_immutable)
		attr |= FS_IMMUTABLE_FL;
	if (supp_nodump)
		attr |= FS_NODUMP_FL;

	ret = ioctl(fd, FS_IOC_SETFLAGS, &attr);
	if (ret < 0)
		tst_brk(TBROK | TERRNO, "ioctl(%i, FS_IOC_SETFLAGS, %i)", fd, attr);

	clear_flags = 1;
}

static void setup(void)
{
	struct statx buf;

	SAFE_MKDIR(TESTDIR_FLAGGED, 0777);
	SAFE_MKDIR(TESTDIR_UNFLAGGED, 0777);

	TEST(statx(AT_FDCWD, TESTDIR_FLAGGED, 0, 0, &buf));
	if (TST_RET == -1)
		tst_brk(TFAIL | TTERRNO,
			"sys_statx(AT_FDCWD, %s, 0, 0, &buf)", TESTDIR_FLAGGED);

	if ((buf.stx_attributes_mask & FS_COMPR_FL) == 0) {
		supp_compr = 0;
		tst_res(TCONF, "filesystem doesn't support FS_COMPR_FL");
	}
	if ((buf.stx_attributes_mask & FS_APPEND_FL) == 0) {
		supp_append = 0;
		tst_res(TCONF, "filesystem doesn't support FS_APPEND_FL");
	}
	if ((buf.stx_attributes_mask & FS_IMMUTABLE_FL) == 0) {
		supp_immutable = 0;
		tst_res(TCONF, "filesystem doesn't support FS_IMMUTABLE_FL");
	}
	if ((buf.stx_attributes_mask & FS_NODUMP_FL) == 0) {
		supp_nodump = 0;
		tst_res(TCONF, "filesystem doesn't support FS_NODUMP_FL");
	}
	if (!(supp_compr || supp_append || supp_immutable || supp_nodump))
		tst_brk(TCONF,
			"filesystem doesn't support the above any attr, skip it");

	caid_flags_setup();
}

static void cleanup(void)
{
	int attr;

	if (clear_flags) {
		SAFE_IOCTL(fd, FS_IOC_GETFLAGS, &attr);
		attr &= ~(FS_COMPR_FL | FS_APPEND_FL | FS_IMMUTABLE_FL | FS_NODUMP_FL);
		SAFE_IOCTL(fd, FS_IOC_SETFLAGS, &attr);
	}

	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test = run,
	.tcnt = 2,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.all_filesystems = 1,
	.mount_device = 1,
	.mntpoint = MOUNT_POINT,
	.min_kver = "4.11",
};
