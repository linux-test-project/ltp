// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2001
 *		07/2001 Ported by Wayne Boyer
 *   Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * Verify that rename() does nothing and returns a success status when
 * oldpath and newpath are existing hard links referring to the same file.
 */

#include <stdio.h>
#include "tst_test.h"

#define MNT_POINT "mntpoint"
#define TEMP_FILE1 MNT_POINT"/tmpfile1"
#define TEMP_FILE2 MNT_POINT"/tmpfile2"

static struct stat buf1, buf2;

static void setup(void)
{
	SAFE_TOUCH(TEMP_FILE1, 0700, NULL);
	SAFE_STAT(TEMP_FILE1, &buf1);
	SAFE_LINK(TEMP_FILE1, TEMP_FILE2);
}

static void run(void)
{
	TST_EXP_PASS(rename(TEMP_FILE1, TEMP_FILE1));
	if (TST_RET != 0)
		return;

	SAFE_STAT(TEMP_FILE2, &buf2);
	TST_EXP_EQ_LU(buf1.st_dev, buf2.st_dev);
	TST_EXP_EQ_LU(buf1.st_ino, buf2.st_ino);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_root = 1,
	.mntpoint = MNT_POINT,
	.mount_device = 1,
	.all_filesystems = 1,
	.skip_filesystems = (const char *const[]){
		"exfat",
		"vfat",
		NULL
	},
};
