// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that rename() fails with EEXIST or ENOTEMPTY when
 * newpath is a non-empty directory.
 */

#include <stdio.h>
#include "tst_test.h"

#define MNT_POINT "mntpoint"
#define DIR1 "dir1"
#define DIR2 "dir2"
#define TEMP_FILE DIR2"/tmpfile"

static void setup(void)
{
	SAFE_CHDIR(MNT_POINT);
	SAFE_MKDIR(DIR1, 00770);
	SAFE_MKDIR(DIR2, 00770);
	SAFE_TOUCH(TEMP_FILE, 0700, NULL);
}

static void run(void)
{
	TEST(rename(DIR1, DIR2));

	if (TST_RET == -1 && (TST_ERR == ENOTEMPTY || TST_ERR == EEXIST))
		tst_res(TPASS | TTERRNO, "rename() failed as expected");
	else if (TST_RET == 0)
		tst_res(TFAIL, "rename() succeeded unexpectedly");
	else
		tst_res(TFAIL | TTERRNO, "rename() failed, but not with expected errno");
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_root = 1,
	.mntpoint = MNT_POINT,
	.mount_device = 1,
	.all_filesystems = 1
};
