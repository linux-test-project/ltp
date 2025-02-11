// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *    Authors: David Fenner, Jon Hendrickson
 * Copyright (C) 2024 Andrea Cervesato andrea.cervesato@suse.com
 */

/*\
 * This test suite validates the behavior of the `rename()` system call on
 * symbolic links under three scenarios:
 *
 * - rename a symlink pointing to an existing file and verifies that the
 *   symlink's inode and device number remain unchanged.
 *
 * - rename a symlink pointing to a non-existent path, ensuring that the
 *   original symlink remains unaffected.
 *
 * - rename a symlink pointing to a created file, confirming that the new
 *   symlink points to the correct file.
 */

#include "tst_test.h"
#include "tst_tmpdir.h"

#define MNTPOINT "mnt"
#define OLDNAME MNTPOINT"/msymlink0"
#define NEWNAME MNTPOINT"/asymlink0"
#define OBJNAME MNTPOINT"/object"

static char *tmpdir;
static char *oldname;
static char *newname;
static char *objname;

static void test_existing(void)
{
	tst_res(TINFO, "Test rename() on symlink pointing to an existent path");

	struct stat buff_stat;
	struct stat oldsym_stat;
	struct stat newsym_stat;

	SAFE_SYMLINK(tmpdir, oldname);
	SAFE_STAT(oldname, &oldsym_stat);

	SAFE_RENAME(oldname, newname);

	TST_EXP_PASS(lstat(newname, &buff_stat));
	TST_EXP_FAIL(lstat(oldname, &buff_stat), ENOENT);

	SAFE_STAT(newname, &newsym_stat);
	TST_EXP_EQ_LI(oldsym_stat.st_ino, newsym_stat.st_ino);
	TST_EXP_EQ_LI(oldsym_stat.st_dev, newsym_stat.st_dev);

	SAFE_UNLINK(newname);
}

static void test_non_existing(void)
{
	tst_res(TINFO, "Test rename() on symlink pointing to a non-existent path");

	struct stat path_stat;

	SAFE_SYMLINK("this_path_doesnt_exist", oldname);
	TST_EXP_FAIL(stat(oldname, &path_stat), ENOENT);
	TST_EXP_PASS(lstat(oldname, &path_stat));

	SAFE_RENAME(oldname, newname);

	TST_EXP_FAIL(lstat(oldname, &path_stat), ENOENT);
	TST_EXP_PASS(lstat(newname, &path_stat));
	TST_EXP_FAIL(stat(newname, &path_stat), ENOENT);

	SAFE_UNLINK(newname);
}

static void test_creat(void)
{
	tst_res(TINFO, "Test rename() on symlink pointing to a path created lately");

	struct stat path_stat;

	SAFE_SYMLINK(objname, oldname);
	TST_EXP_FAIL(stat(oldname, &path_stat), ENOENT);
	TST_EXP_PASS(lstat(oldname, &path_stat));

	tst_res(TINFO, "Create object file");

	int fd;

	fd = SAFE_CREAT(objname, 0700);
	if (fd >= 0)
		SAFE_CLOSE(fd);

	SAFE_RENAME(oldname, newname);

	TST_EXP_PASS(lstat(newname, &path_stat));
	TST_EXP_PASS(stat(newname, &path_stat));

	TST_EXP_FAIL(lstat(oldname, &path_stat), ENOENT);
	TST_EXP_PASS(stat(objname, &path_stat));

	SAFE_UNLINK(objname);
	SAFE_UNLINK(newname);
}

static void run(void)
{
	test_existing();
	test_creat();
	test_non_existing();
}

static void setup(void)
{
	tmpdir = tst_tmpdir_path();
	oldname = tst_tmpdir_genpath(OLDNAME);
	newname = tst_tmpdir_genpath(NEWNAME);
	objname = tst_tmpdir_genpath(OBJNAME);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.all_filesystems = 1,
	.mntpoint = MNTPOINT,
	.format_device = 1,
	.needs_root = 1,
};
