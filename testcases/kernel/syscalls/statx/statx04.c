// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Zilogic Systems Pvt. Ltd., 2018
 * Email: code@zilogic.com
 */

/*
 * Test statx
 *
 * This code tests if the attributes field of statx received expected value.
 * File set with following flags by using SAFE_IOCTL:
 * 1) STATX_ATTR_COMPRESSED - The file is compressed by the filesystem.
 * 2) STATX_ATTR_IMMUTABLE - The file cannot be modified.
 * 3) STATX_ATTR_APPEND - The file can only be opened in append mode for
 *                        writing.
 * 4) STATX_ATTR_NODUMP - File is not a candidate for backup when a backup
 *                        program such as dump(8) is run.
 *
 * Two directories are tested.
 * First directory has all flags set.
 * Second directory has no flags set.
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

static void test_flagged(void)
{
	struct statx buf;

	TEST(statx(AT_FDCWD, TESTDIR_FLAGGED, 0, 0, &buf));
	if (TST_RET == 0)
		tst_res(TPASS,
			"sys_statx(AT_FDCWD, %s, 0, 0, &buf)", TESTDIR_FLAGGED);
	else
		tst_brk(TFAIL | TTERRNO,
			"sys_statx(AT_FDCWD, %s, 0, 0, &buf)", TESTDIR_FLAGGED);

	if (buf.stx_attributes & STATX_ATTR_COMPRESSED)
		tst_res(TPASS, "STATX_ATTR_COMPRESSED flag is set");
	else
		tst_res(TFAIL, "STATX_ATTR_COMPRESSED flag is not set");

	if (buf.stx_attributes & STATX_ATTR_APPEND)
		tst_res(TPASS, "STATX_ATTR_APPEND flag is set");
	else
		tst_res(TFAIL, "STATX_ATTR_APPEND flag is not set");

	if (buf.stx_attributes & STATX_ATTR_IMMUTABLE)
		tst_res(TPASS, "STATX_ATTR_IMMUTABLE flag is set");
	else
		tst_res(TFAIL, "STATX_ATTR_IMMUTABLE flag is not set");

	if (buf.stx_attributes & STATX_ATTR_NODUMP)
		tst_res(TPASS, "STATX_ATTR_NODUMP flag is set");
	else
		tst_res(TFAIL, "STATX_ATTR_NODUMP flag is not set");
}

static void test_unflagged(void)
{
	struct statx buf;

	TEST(statx(AT_FDCWD, TESTDIR_UNFLAGGED, 0, 0, &buf));
	if (TST_RET == 0)
		tst_res(TPASS,
			"sys_statx(AT_FDCWD, %s, 0, 0, &buf)",
			TESTDIR_UNFLAGGED);
	else
		tst_brk(TFAIL | TTERRNO,
			"sys_statx(AT_FDCWD, %s, 0, 0, &buf)",
			TESTDIR_UNFLAGGED);

	if ((buf.stx_attributes & STATX_ATTR_COMPRESSED) == 0)
		tst_res(TPASS, "STATX_ATTR_COMPRESSED flag is not set");
	else
		tst_res(TFAIL, "STATX_ATTR_COMPRESSED flag is set");

	if ((buf.stx_attributes & STATX_ATTR_APPEND) == 0)
		tst_res(TPASS, "STATX_ATTR_APPEND flag is not set");
	else
		tst_res(TFAIL, "STATX_ATTR_APPEND flag is set");

	if ((buf.stx_attributes & STATX_ATTR_IMMUTABLE) == 0)
		tst_res(TPASS, "STATX_ATTR_IMMUTABLE flag is not set");
	else
		tst_res(TFAIL, "STATX_ATTR_IMMUTABLE flag is set");

	if ((buf.stx_attributes & STATX_ATTR_NODUMP) == 0)
		tst_res(TPASS, "STATX_ATTR_NODUMP flag is not set");
	else
		tst_res(TFAIL, "STATX_ATTR_NODUMP flag is set");
}

struct test_cases {
	void (*tfunc)(void);
} tcases[] = {
	{&test_flagged},
	{&test_unflagged},
};

static void run(unsigned int i)
{
	tcases[i].tfunc();
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

	attr |= FS_COMPR_FL | FS_APPEND_FL | FS_IMMUTABLE_FL | FS_NODUMP_FL;

	ret = ioctl(fd, FS_IOC_SETFLAGS, &attr);
	if (ret < 0) {
		if (errno == EOPNOTSUPP)
			tst_brk(TCONF, "Flags not supported");
		tst_brk(TBROK | TERRNO, "ioctl(%i, FS_IOC_SETFLAGS, %i)", fd, attr);
	}

	clear_flags = 1;
}

static void setup(void)
{
	SAFE_MKDIR(TESTDIR_FLAGGED, 0777);
	SAFE_MKDIR(TESTDIR_UNFLAGGED, 0777);

	if (!strcmp(tst_device->fs_type, "btrfs") && tst_kvercmp(4, 13, 0) < 0)
		tst_brk(TCONF, "Btrfs statx() supported since 4.13");

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
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.all_filesystems = 1,
	.mount_device = 1,
	.mntpoint = MOUNT_POINT,
	.min_kver = "4.11",
};
