// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * Verify that rename(2) fails with EISDIR when
 * oldpath is not a directory and newpath is an existing directory.
 */

#include <stdio.h>
#include "tst_test.h"

#define MNT_POINT "mntpoint"
#define TEMP_FILE "tmpfile"
#define TEMP_DIR "tmpdir"

static void setup(void)
{
	SAFE_CHDIR(MNT_POINT);
	SAFE_TOUCH(TEMP_FILE, 0700, NULL);
	SAFE_MKDIR(TEMP_DIR, 00770);
}

static void run(void)
{
	TST_EXP_FAIL(rename(TEMP_FILE, TEMP_DIR),
				EISDIR);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MNT_POINT,
	.all_filesystems = 1
};
