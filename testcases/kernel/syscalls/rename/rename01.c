// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify rename() when the newpath file or directory does not exist.
 */

#include <stdio.h>
#include "tst_test.h"

#define MNT_POINT "mntpoint"

static const char *old_file_name = "oldfile";
static const char *old_dir_name = "olddir";
static const char *new_file_name = "newfile";
static const char *new_dir_name = "newdir";

static struct stat old_file_st, old_dir_st, new_file_st, new_dir_st;

static inline void swap(const char **a, const char **b)
{
	const char *tmp__ = *a;
	*a = *b;
	*b = tmp__;
}

static void setup(void)
{
	SAFE_CHDIR(MNT_POINT);

	SAFE_TOUCH(old_file_name, 0700, NULL);
	SAFE_MKDIR(old_dir_name, 00770);

	SAFE_STAT(old_file_name, &old_file_st);
	SAFE_STAT(old_dir_name, &old_dir_st);
}

static void run(void)
{
	TST_EXP_PASS(rename(old_file_name, new_file_name),
						"rename(%s, %s)",
						old_file_name, new_file_name);
	TST_EXP_PASS(rename(old_dir_name, new_dir_name),
						"rename(%s, %s)",
						old_dir_name, new_dir_name);

	SAFE_STAT(new_file_name, &new_file_st);
	SAFE_STAT(new_dir_name, &new_dir_st);

	TST_EXP_EQ_LU(old_file_st.st_dev, new_file_st.st_dev);
	TST_EXP_EQ_LU(old_file_st.st_ino, new_file_st.st_ino);

	TST_EXP_EQ_LU(old_dir_st.st_dev, new_dir_st.st_dev);
	TST_EXP_EQ_LU(old_dir_st.st_ino, new_dir_st.st_ino);

	TST_EXP_FAIL(stat(old_file_name, &old_file_st),
				ENOENT,
				"stat(%s, &old_file_st)",
				old_file_name);
	TST_EXP_FAIL(stat(old_dir_name, &old_dir_st),
				ENOENT,
				"stat(%s, &old_dir_st)",
				old_dir_name);

	/* reset between loops */
	swap(&old_file_name, &new_file_name);
	swap(&old_dir_name, &new_dir_name);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MNT_POINT,
	.all_filesystems = 1
};
