// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2012  Red Hat, Inc.
 * Copyright (c) 2023 Marius Kittler <mkittler@suse.de>
 */

/*\
 * [Description]
 *
 * An empty buffer of size zero can be passed into getxattr(2) to
 * return the current size of the named extended attribute.
 */

#include "config.h"
#include "tst_test.h"

#include <sys/xattr.h>
#include "tst_safe_macros.h"

#define MNTPOINT "mntpoint"
#define FNAME MNTPOINT"/getxattr03testfile"
#define XATTR_TEST_KEY "user.testkey"
#define XATTR_TEST_VALUE "test value"
#define XATTR_TEST_VALUE_SIZE (sizeof(XATTR_TEST_VALUE) - 1)

static void run(void)
{
	TST_EXP_VAL(getxattr(FNAME, XATTR_TEST_KEY, NULL, 0),
				XATTR_TEST_VALUE_SIZE);
}

static void setup(void)
{
	SAFE_TOUCH(FNAME, 0644, NULL);
	SAFE_SETXATTR(FNAME, XATTR_TEST_KEY, XATTR_TEST_VALUE,
		     XATTR_TEST_VALUE_SIZE, XATTR_CREATE);
}

static struct tst_test test = {
	.all_filesystems = 1,
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.mount_device = 1,
	.skip_filesystems = (const char *const []) {
			"exfat",
			"tmpfs",
			"ramfs",
			"nfs",
			"vfat",
			NULL
	},
	.setup = setup,
	.test_all = run,
};
