// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Linaro Limited. All rights reserved.
 * Author: Rafael David Tinoco <rafael.tinoco@linaro.org>
 */

/*
 * Test Name: fremovexattr01
 *
 * Description:
 * Like removexattr(2), fremovexattr(2) also removes an extended attribute,
 * identified by a name, from a file but, instead of using a filename path, it
 * uses a descriptor. This test verifies that a simple call to fremovexattr(2)
 * removes, indeed, a previously set attribute key/value from a file.
 */

#include "config.h"
#include <errno.h>
#include <stdlib.h>

#ifdef HAVE_SYS_XATTR_H
# include <sys/xattr.h>
#endif

#include "tst_test.h"

#ifdef HAVE_SYS_XATTR_H

#define ENOATTR ENODATA

#define XATTR_TEST_KEY "user.testkey"
#define XATTR_TEST_VALUE "this is a test value"
#define XATTR_TEST_VALUE_SIZE 20

#define MNTPOINT "mntpoint"
#define FNAME MNTPOINT"/fremovexattr01testfile"

static int fd = -1;
static char got_value[XATTR_TEST_VALUE_SIZE];

static void verify_fremovexattr(void)
{
	SAFE_FSETXATTR(fd, XATTR_TEST_KEY, XATTR_TEST_VALUE,
			XATTR_TEST_VALUE_SIZE, XATTR_CREATE);

	TEST(fremovexattr(fd, XATTR_TEST_KEY));

	if (TST_RET != 0) {
		tst_res(TFAIL | TTERRNO, "fremovexattr(2) failed");
		return;
	}

	TEST(fgetxattr(fd, XATTR_TEST_KEY, got_value, sizeof(got_value)));

	if (TST_RET >= 0) {
		tst_res(TFAIL, "fremovexattr(2) did not remove attribute");
		return;
	}

	if (TST_RET < 0 && TST_ERR != ENOATTR) {
		tst_brk(TBROK | TTERRNO,
			"fremovexattr(2) could not verify removal");
		return;
	}

	tst_res(TPASS, "fremovexattr(2) removed attribute as expected");
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static void setup(void)
{
	fd = SAFE_OPEN(FNAME, O_RDWR | O_CREAT, 0644);

	TEST(fremovexattr(fd, XATTR_TEST_KEY));

	if (TST_RET == -1 && TST_ERR == EOPNOTSUPP)
		tst_brk(TCONF, "fremovexattr(2) not supported");
}

static struct tst_test test = {
	.timeout = 12,
	.setup = setup,
	.test_all = verify_fremovexattr,
	.cleanup = cleanup,
	.mntpoint = MNTPOINT,
	.mount_device = 1,
	.all_filesystems = 1,
	.needs_root = 1,
};

#else /* HAVE_SYS_XATTR_H */
TST_TEST_TCONF("<sys/xattr.h> does not exist");
#endif
