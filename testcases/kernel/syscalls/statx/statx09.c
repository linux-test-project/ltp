// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 FUJITSU LIMITED. All rights reserved.
 * Author: Dai Shili <daisl.fnst@fujitsu.com>
 */

/*\
 * [Description]
 *
 * This code tests if STATX_ATTR_VERITY flag in the statx attributes is set correctly.
 *
 * The statx() system call sets STATX_ATTR_VERITY if the file has fs-verity
 * enabled. This can perform better than FS_IOC_GETFLAGS and
 * FS_IOC_MEASURE_VERITY because it doesn't require opening the file, and
 * opening verity files can be expensive.
 *
 * Minimum Linux version required is v5.5.
 */

#define _GNU_SOURCE
#include <sys/mount.h>
#include <stdlib.h>
#include "tst_test.h"
#include "lapi/fs.h"
#include "lapi/fsverity.h"
#include "lapi/stat.h"
#include "lapi/fcntl.h"
#include <inttypes.h>

#define MNTPOINT "mnt_point"
#define TESTFILE_FLAGGED MNTPOINT"/test_file1"
#define TESTFILE_UNFLAGGED MNTPOINT"/test_file2"

static int mount_flag;

static const uint32_t hash_algorithms[] = {
	FS_VERITY_HASH_ALG_SHA256,
};

static void test_flagged(void)
{
	struct statx buf;

	TST_EXP_PASS(statx(AT_FDCWD, TESTFILE_FLAGGED, 0, 0, &buf),
		"statx(AT_FDCWD, %s, 0, 0, &buf)", TESTFILE_FLAGGED);

	if (buf.stx_attributes & STATX_ATTR_VERITY)
		tst_res(TPASS, "STATX_ATTR_VERITY flag is set: (%"PRIu64") ",
			(uint64_t)buf.stx_attributes);
	else
		tst_res(TFAIL, "STATX_ATTR_VERITY flag is not set");
}

static void test_unflagged(void)
{
	struct statx buf;

	TST_EXP_PASS(statx(AT_FDCWD, TESTFILE_UNFLAGGED, 0, 0, &buf),
		"statx(AT_FDCWD, %s, 0, 0, &buf)", TESTFILE_UNFLAGGED);

	if ((buf.stx_attributes & STATX_ATTR_VERITY) == 0)
		tst_res(TPASS, "STATX_ATTR_VERITY flag is not set");
	else
		tst_res(TFAIL, "STATX_ATTR_VERITY flag is set");
}

static struct test_cases {
	void (*tfunc)(void);
} tcases[] = {
	{&test_flagged},
	{&test_unflagged},
};

static void run(unsigned int i)
{
	tcases[i].tfunc();
}

static void flag_setup(void)
{
	int fd, attr, ret;
	struct fsverity_enable_arg enable;
	struct stat statbuf;

	fd = SAFE_OPEN(TESTFILE_FLAGGED, O_RDONLY, 0664);
	SAFE_FSTAT(fd, &statbuf);

	ret = ioctl(fd, FS_IOC_GETFLAGS, &attr);
	if (ret < 0) {
		if (errno == ENOTTY)
			tst_brk(TCONF | TERRNO, "FS_IOC_GETFLAGS not supported");

		tst_brk(TBROK | TERRNO, "ioctl(%i, FS_IOC_GETFLAGS, ...)", fd);
	}

	memset(&enable, 0, sizeof(enable));
	enable.version = 1;
	enable.hash_algorithm = hash_algorithms[0];
	enable.block_size = statbuf.st_blksize;
	enable.salt_size = 0;
	enable.salt_ptr = (intptr_t)NULL;
	enable.sig_size = 0;
	enable.sig_ptr = (intptr_t)NULL;

	ret = ioctl(fd, FS_IOC_ENABLE_VERITY, &enable);
	if (ret < 0) {
		if (errno == EOPNOTSUPP) {
			tst_brk(TCONF,
				"fs-verity is not supported on the file system or by the kernel");
		}
		tst_brk(TBROK | TERRNO, "ioctl(%i, FS_IOC_ENABLE_VERITY) failed", fd);
	}

	ret = ioctl(fd, FS_IOC_GETFLAGS, &attr);
	if ((ret == 0) && !(attr & FS_VERITY_FL))
		tst_res(TFAIL, "%i: fs-verity enabled but FS_VERITY_FL bit not set", fd);

	SAFE_CLOSE(fd);
}

static void setup(void)
{
	TEST(mount(tst_device->dev, MNTPOINT, tst_device->fs_type, 0, NULL));
	if (TST_RET) {
		if (TST_ERR == EINVAL)
			tst_brk(TCONF, "fs-verity not supported on loopdev");

		tst_brk(TBROK | TERRNO, "mount() failed with %ld", TST_RET);
	}
	mount_flag = 1;

	SAFE_FILE_PRINTF(TESTFILE_FLAGGED, "a");
	SAFE_FILE_PRINTF(TESTFILE_UNFLAGGED, "a");

	flag_setup();
}

static void cleanup(void)
{
	if (mount_flag)
		tst_umount(MNTPOINT);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.format_device = 1,
	.dev_fs_type = "ext4",
	.dev_fs_opts = (const char *const []){"-O verity", NULL},
	.needs_kconfigs = (const char *[]) {
		"CONFIG_FS_VERITY",
		NULL
	},
	.needs_cmds = (const char *[]) {
		"mkfs.ext4 >= 1.45.2",
		NULL
	}
};
