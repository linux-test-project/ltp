// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2013 Fujitsu Ltd. Dan Li <li.dan@cn.fujitsu.com>
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test for feature MS_BIND of mount, which performs a bind mount, making a file
 * or a directory subtree visible at another point within a file system.
 */

#include "tst_test.h"
#include <sys/mount.h>

#define MNTPOINT1 "mntpoint1"
#define TESTFILE1 MNTPOINT1 "/testfile"
#define TESTDIR1  MNTPOINT1 "/testdir"

#define MNTPOINT2 "mntpoint2"
#define TESTFILE2 MNTPOINT2 "/testfile"
#define TESTDIR2  MNTPOINT2 "/testdir"

static void setup(void)
{
	SAFE_MOUNT(tst_device->dev, MNTPOINT1, tst_device->fs_type, 0, NULL);

	tst_res(TINFO, "Creating file in '%s'", TESTFILE1);

	SAFE_FILE_PRINTF(TESTFILE1, "LTP TEST FILE");
	SAFE_MKDIR(TESTDIR1, 0750);
}

static void cleanup(void)
{
	if (tst_is_mounted(MNTPOINT1))
		SAFE_UMOUNT(MNTPOINT1);

	if (!access(MNTPOINT2, F_OK) && tst_is_mounted(MNTPOINT2))
		SAFE_UMOUNT(MNTPOINT2);
}

static void run(void)
{
	SAFE_MKDIR(MNTPOINT2, 0750);
	SAFE_MOUNT(MNTPOINT1, MNTPOINT2, tst_device->fs_type, MS_BIND, NULL);

	TST_EXP_PASS(access(TESTFILE2, F_OK), "Accessing to '%s'", TESTFILE2);
	TST_EXP_PASS(access(TESTDIR2, F_OK), "Accessing to '%s'", TESTDIR2);

	if (tst_is_mounted(MNTPOINT2))
		SAFE_UMOUNT(MNTPOINT2);

	SAFE_RMDIR(MNTPOINT2);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
	.needs_root = 1,
	.format_device = 1,
	.mntpoint = MNTPOINT1,
	.all_filesystems = 1,
	.skip_filesystems = (const char *const []){
		"exfat",
		"vfat",
		"ntfs",
		NULL
	},
};
