// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Linaro Limited. All rights reserved.
 * Author: Rafael David Tinoco <rafael.tinoco@linaro.org>
 */

/*\
 * lremovexattr(2) removes the extended attribute identified by a name and
 * associated with a given path in the filesystem. Unlike removexattr(2),
 * lremovexattr(2) removes the attribute from the symbolic link only, and not
 * the file. This test verifies that a simple call to lremovexattr(2) removes,
 * indeed, a previously set attribute key/value from a symbolic link, and the
 * symbolic link _only_.
 *
 * Note:
 * According to attr(5), extended attributes are interpreted differently from
 * regular files, directories and symbolic links. User attributes are only
 * allowed for regular files and directories, thus the need of using trusted.*
 * attributes for this test.
 */

#include "config.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_SYS_XATTR_H
# include <sys/xattr.h>
#endif

#include "tst_test.h"

#ifdef HAVE_SYS_XATTR_H

#define ENOATTR ENODATA

#define XATTR_KEY		"trusted.key1"
#define XATTR_VALUE		"file and link"
#define XATTR_VALUE_SIZE	13

#define MNTPOINT "mntpoint"
#define FILENAME MNTPOINT"/lremovexattr01testfile"
#define SYMLINK  MNTPOINT"/lremovexattr01symlink"

static char got_value[XATTR_VALUE_SIZE];

static void verify_lremovexattr(void)
{
	/* set attribute on both: file and symlink */

	SAFE_SETXATTR(FILENAME, XATTR_KEY, XATTR_VALUE, XATTR_VALUE_SIZE,
			XATTR_CREATE);

	SAFE_LSETXATTR(SYMLINK, XATTR_KEY, XATTR_VALUE, XATTR_VALUE_SIZE,
			XATTR_CREATE);

	/* remove attribute from symlink only */

	TEST(lremovexattr(SYMLINK, XATTR_KEY));

	if (TST_RET != 0) {
		tst_res(TFAIL | TTERRNO, "lremovexattr(2) failed");
		return;
	}

	/* make sure attribute is gone from symlink */

	memset(&got_value, 0, XATTR_VALUE_SIZE);

	TEST(lgetxattr(SYMLINK, XATTR_KEY, &got_value, XATTR_VALUE_SIZE));

	if (TST_RET >= 0) {
		tst_res(TFAIL, "lremovexattr(2) did not work");
		return;
	}

	if (TST_ERR != ENOATTR) {
		tst_brk(TBROK | TTERRNO, "lgetxattr(2) failed unexpectedly");
		return;
	}

	/* check if file is unchanged, like it should be */

	memset(&got_value, 0, XATTR_VALUE_SIZE);

	TEST(getxattr(FILENAME, XATTR_KEY, &got_value, XATTR_VALUE_SIZE));

	if (TST_RET <= 0) {
		tst_res(TFAIL, "lremovexattr(2) deleted file attribute");
		return;
	}

	if (strncmp(got_value, XATTR_VALUE, XATTR_VALUE_SIZE)) {
		tst_res(TFAIL, "lremovexattr(2) changed file attribute");
		return;
	}

	/* cleanup file attribute for iteration */

	SAFE_REMOVEXATTR(FILENAME, XATTR_KEY);

	tst_res(TPASS, "lremovexattr(2) removed attribute as expected");
}

static void setup(void)
{
	SAFE_TOUCH(FILENAME, 0644, NULL);

	if (symlink(FILENAME, SYMLINK) < 0)
		tst_brk(TCONF, "symlink() not supported");
}

static struct tst_test test = {
	.setup = setup,
	.test_all = verify_lremovexattr,
	.mntpoint = MNTPOINT,
	.mount_device = 1,
	.all_filesystems = 1,
	.needs_root = 1,
};

#else /* HAVE_SYS_XATTR_H */
TST_TEST_TCONF("<sys/xattr.h> does not exist");
#endif
