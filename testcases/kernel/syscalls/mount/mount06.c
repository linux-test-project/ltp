// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2013 Fujitsu Ltd. Dan Li <li.dan@cn.fujitsu.com>
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Test for feature MS_MOVE of mount, which moves an existing mount point to
 * a new location.
 */

#include "tst_test.h"
#include <stdio.h>
#include <sys/mount.h>

#ifndef MS_MOVE
#define MS_MOVE	8192
#endif

#ifndef MS_PRIVATE
#define MS_PRIVATE	(1 << 18)
#endif

#define MNTPOINT_SRC "mntpoint1"
#define MNTPOINT_DST "mntpoint2"

static char *tmppath;
static char *mntpoint_src;
static char *mntpoint_dst;
static char *tstfiles_src;
static char *tstfiles_dst;

static void setup(void)
{
	tmppath = tst_tmpdir_path();

	/*
	 * Turn current dir into a private mount point being a parent
	 * mount which is required by move mount.
	 */
	SAFE_MOUNT(tmppath, tmppath, "none", MS_BIND, NULL);
	SAFE_MOUNT("none", tmppath, "none", MS_PRIVATE, NULL);

	mntpoint_src = tst_tmpdir_mkpath(MNTPOINT_SRC);
	mntpoint_dst = tst_tmpdir_mkpath(MNTPOINT_DST);
	tstfiles_src = tst_tmpdir_mkpath("%s/testfile", MNTPOINT_SRC);
	tstfiles_dst = tst_tmpdir_mkpath("%s/testfile", MNTPOINT_DST);

	SAFE_MKDIR(mntpoint_dst, 0750);
}

static void cleanup(void)
{
	if (tst_is_mounted(mntpoint_src))
		SAFE_UMOUNT(mntpoint_src);

	if (tst_is_mounted(mntpoint_dst))
		SAFE_UMOUNT(mntpoint_dst);

	if (tst_is_mounted(tmppath))
		SAFE_UMOUNT(tmppath);

	SAFE_RMDIR(mntpoint_dst);
}

static void run(void)
{
	SAFE_MOUNT(tst_device->dev, mntpoint_src, tst_device->fs_type, 0, NULL);
	SAFE_FILE_PRINTF(tstfiles_src, "LTP TEST FILE");

	SAFE_MOUNT(mntpoint_src, mntpoint_dst, tst_device->fs_type, MS_MOVE, NULL);

	TST_EXP_FAIL(
		access(tstfiles_src, F_OK),
		ENOENT,
		"File %s doesn't exist",
		tstfiles_src);

	TST_EXP_PASS(
		access(tstfiles_dst, F_OK),
		"File %s exists :",
		tstfiles_dst);

	if (tst_is_mounted(mntpoint_src))
		SAFE_UMOUNT(mntpoint_src);

	if (tst_is_mounted(mntpoint_dst))
		SAFE_UMOUNT(mntpoint_dst);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
	.needs_root = 1,
	.format_device = 1,
	.mntpoint = MNTPOINT_SRC,
	.all_filesystems = 1,
	.skip_filesystems = (const char *const []){
		"exfat",
		"vfat",
		"ntfs",
		NULL
	},
};
