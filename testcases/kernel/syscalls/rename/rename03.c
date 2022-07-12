// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify rename(2) functions correctly when the newpath
 * file or directory (empty) exists.
 */

#include <sys/stat.h>
#include <stdio.h>
#include "tst_test.h"

#define MNT_POINT "mntpoint"
#define OLD_FILE_NAME MNT_POINT"/oldfile"
#define NEW_FILE_NAME MNT_POINT"/newfile"
#define OLD_DIR_NAME MNT_POINT"/olddir"
#define NEW_DIR_NAME MNT_POINT"/newdir"

static struct stat old_file_st, old_dir_st, new_file_st, new_dir_st;

static void run(void)
{
	SAFE_TOUCH(OLD_FILE_NAME, 0700, NULL);
	SAFE_MKDIR(OLD_DIR_NAME, 00770);
	SAFE_TOUCH(NEW_FILE_NAME, 0700, NULL);
	SAFE_MKDIR(NEW_DIR_NAME, 00770);

	SAFE_STAT(OLD_FILE_NAME, &old_file_st);
	SAFE_STAT(OLD_DIR_NAME, &old_dir_st);

	TST_EXP_PASS(rename(OLD_FILE_NAME, NEW_FILE_NAME),
						"rename(%s, %s)",
						OLD_FILE_NAME, NEW_FILE_NAME);
	TST_EXP_PASS(rename(OLD_DIR_NAME, NEW_DIR_NAME),
						"rename(%s, %s)",
						OLD_DIR_NAME, NEW_DIR_NAME);

	SAFE_STAT(NEW_FILE_NAME, &new_file_st);
	SAFE_STAT(NEW_DIR_NAME, &new_dir_st);

	TST_EXP_EQ_LU(old_file_st.st_dev, new_file_st.st_dev);
	TST_EXP_EQ_LU(old_file_st.st_ino, new_file_st.st_ino);

	TST_EXP_EQ_LU(old_dir_st.st_dev, new_dir_st.st_dev);
	TST_EXP_EQ_LU(old_dir_st.st_ino, new_dir_st.st_ino);

	TST_EXP_FAIL(stat(OLD_FILE_NAME, &old_file_st),
				ENOENT,
				"stat(%s, &old_file_st)",
				OLD_FILE_NAME);
	TST_EXP_FAIL(stat(OLD_DIR_NAME, &old_dir_st),
				ENOENT,
				"stat(%s, &old_dir_st)",
				OLD_DIR_NAME);

	/* cleanup between loops */
	SAFE_UNLINK(NEW_FILE_NAME);
	SAFE_RMDIR(NEW_DIR_NAME);
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MNT_POINT,
	.all_filesystems = 1
};
