// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that rename(2) fails with EINVAL when
 * an attempt is made to make a directory a subdirectory of itself.
 */

#include <stdio.h>
#include "tst_test.h"

#define MNT_POINT "mntpoint"
#define DIR1 "dir1"
#define DIR2 DIR1"/dir2"

static void setup(void)
{
	SAFE_CHDIR(MNT_POINT);
	SAFE_MKDIR(DIR1, 00770);
	SAFE_MKDIR(DIR2, 00770);
}

static void run(void)
{
	TST_EXP_FAIL(rename(DIR1, DIR2),
				EINVAL);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MNT_POINT,
	.all_filesystems = 1
};
