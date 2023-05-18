// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Zilogic Systems Pvt. Ltd., 2018
 * Email: code@zilogic.com
 */

/*\
 * [Description]
 *
 * Test statx syscall with STATX_ATTR_ENCRYPTED flag, setting a key is required
 * for the file to be encrypted by the filesystem.
 *
 * e4crypt is used to set the encrypt flag (currently supported only by ext4).
 *
 * Two directories are tested.
 * First directory has all flags set.
 * Second directory has no flags set.
 *
 * Minimum e2fsprogs version required is 1.43.
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "tst_test.h"
#include "lapi/fs.h"
#include "lapi/stat.h"
#include "lapi/fcntl.h"

#define MNTPOINT "mnt_point"
#define TESTDIR_FLAGGED MNTPOINT"/test_dir1"
#define TESTDIR_UNFLAGGED MNTPOINT"/test_dir2"

static int mount_flag;

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

	if (buf.stx_attributes & STATX_ATTR_ENCRYPTED)
		tst_res(TPASS, "STATX_ATTR_ENCRYPTED flag is set");
	else
		tst_res(TFAIL, "STATX_ATTR_ENCRYPTED flag is not set");
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

	if ((buf.stx_attributes & STATX_ATTR_ENCRYPTED) == 0)
		tst_res(TPASS, "STATX_ATTR_ENCRYPTED flag is not set");
	else
		tst_res(TFAIL, "STATX_ATTR_ENCRYPTED flag is set");
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

static void setup(void)
{
	char opt_bsize[32];
	const char *const fs_opts[] = {"-O encrypt", opt_bsize, NULL};
	int ret;

	snprintf(opt_bsize, sizeof(opt_bsize), "-b %i", getpagesize());

	SAFE_MKFS(tst_device->dev, tst_device->fs_type, fs_opts, NULL);
	SAFE_MOUNT(tst_device->dev, MNTPOINT, tst_device->fs_type, 0, 0);
	mount_flag = 1;

	SAFE_MKDIR(TESTDIR_FLAGGED, 0777);
	SAFE_MKDIR(TESTDIR_UNFLAGGED, 0777);

	ret = tst_system("echo qwery | e4crypt add_key "TESTDIR_FLAGGED);

	if (ret)
		tst_brk(TCONF, "e4crypt failed (CONFIG_EXT4_ENCRYPTION not set?)");
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
	.min_kver = "4.11",
	.needs_root = 1,
	.needs_device = 1,
	.mntpoint = MNTPOINT,
	.dev_fs_type = "ext4",
	.needs_cmds = (const char *[]) {
		"mkfs.ext4 >= 1.43.0",
		"e4crypt",
		NULL
	}
};
