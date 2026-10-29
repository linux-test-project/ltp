// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * Verify that rename(2) fails with ENAMETOOLONG, when
 * oldpath or newpath is too long.
 */

#include <stdio.h>
#include "tst_test.h"

#define MNT_POINT "mntpoint"
#define TEMP_FILE "tmpfile"

/* Path longer than PATH_MAX: fails the syscall right away (getname() fails) */
static char long_path[PATH_MAX + 1] = {[0 ... PATH_MAX] = 'a'};
/*
 * Path fitting in PATH_MAX, but with an excessively long file name: rejected
 * by the underlying filesystem
 */
static char long_name[PATH_MAX] = {[0 ... PATH_MAX - 2] = 'a', [PATH_MAX - 1] = '\0'};

static void setup(void)
{
	SAFE_CHDIR(MNT_POINT);
	SAFE_TOUCH(TEMP_FILE, 0700, NULL);
}

static void run(void)
{
	TST_EXP_FAIL(rename(TEMP_FILE, long_path),
				ENAMETOOLONG);
	TST_EXP_FAIL(rename(TEMP_FILE, long_name),
				ENAMETOOLONG);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MNT_POINT,
	.all_filesystems = 1
};
