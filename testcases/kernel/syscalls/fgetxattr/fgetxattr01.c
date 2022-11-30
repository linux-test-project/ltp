// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Linaro Limited. All rights reserved.
 * Author: Rafael David Tinoco <rafael.tinoco@linaro.org>
 */

/*
 * Basic tests for fgetxattr(2) and make sure fgetxattr(2) handles error
 * conditions correctly.
 *
 * There are 3 test cases:
 * 1. Get an non-existing attribute:
 *     - fgetxattr(2) should return -1 and set errno to ENODATA
 * 2. Buffer size is smaller than attribute value size:
 *     - fgetxattr(2) should return -1 and set errno to ERANGE
 * 3. Get attribute, fgetxattr(2) should succeed:
 *     - verify the attribute got by fgetxattr(2) is same as the value we set
 */

#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_SYS_XATTR_H
# include <sys/xattr.h>
#endif
#include "tst_test.h"

#ifdef HAVE_SYS_XATTR_H
#define XATTR_SIZE_MAX 65536
#define XATTR_TEST_KEY "user.testkey"
#define XATTR_TEST_VALUE "this is a test value"
#define XATTR_TEST_VALUE_SIZE 20
#define XATTR_TEST_INVALID_KEY "user.nosuchkey"
#define MNTPOINT "mntpoint"
#define FNAME MNTPOINT"/fgetxattr01testfile"

static int fd = -1;

struct test_case {
	char *key;
	char *value;
	size_t size;
	int exp_ret;
	int exp_err;
};
struct test_case tc[] = {
	{			/* case 00, get non-existing attribute */
	 .key = XATTR_TEST_INVALID_KEY,
	 .value = NULL,
	 .size = XATTR_SIZE_MAX,
	 .exp_ret = -1,
	 .exp_err = ENODATA,
	 },
	{			/* case 01, small value buffer */
	 .key = XATTR_TEST_KEY,
	 .value = NULL,
	 .size = 1,
	 .exp_ret = -1,
	 .exp_err = ERANGE,
	 },
	{			/* case 02, get existing attribute */
	 .key = XATTR_TEST_KEY,
	 .value = NULL,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .exp_ret = XATTR_TEST_VALUE_SIZE,
	 .exp_err = 0,
	 },
};

static void verify_fgetxattr(unsigned int i)
{
	TEST(fgetxattr(fd, tc[i].key, tc[i].value, tc[i].size));

	if (TST_RET == -1 && TST_ERR == EOPNOTSUPP)
		tst_brk(TCONF, "fgetxattr(2) not supported");

	if (TST_RET >= 0) {

		if (tc[i].exp_ret == TST_RET)
			tst_res(TPASS, "fgetxattr(2) passed");
		else
			tst_res(TFAIL, "fgetxattr(2) passed unexpectedly");

		if (strncmp(tc[i].value, XATTR_TEST_VALUE,
				XATTR_TEST_VALUE_SIZE)) {
			tst_res(TFAIL, "wrong value, expect \"%s\" got \"%s\"",
					 XATTR_TEST_VALUE, tc[i].value);
		}

		tst_res(TPASS, "got the right value");
	}

	if (tc[i].exp_err == TST_ERR) {
		tst_res(TPASS | TTERRNO, "fgetxattr(2) passed");
		return;
	}

	tst_res(TFAIL | TTERRNO, "fgetxattr(2) failed");
}

static void setup(void)
{
	size_t i = 0;

	SAFE_TOUCH(FNAME, 0644, NULL);
	fd = SAFE_OPEN(FNAME, O_RDONLY);

	for (i = 0; i < ARRAY_SIZE(tc); i++) {
		tc[i].value = SAFE_MALLOC(tc[i].size);
		memset(tc[i].value, 0, tc[i].size);
	}

	SAFE_FSETXATTR(fd, XATTR_TEST_KEY, XATTR_TEST_VALUE,
			XATTR_TEST_VALUE_SIZE, XATTR_CREATE);
}

static void cleanup(void)
{
	size_t i = 0;

	for (i = 0; i < ARRAY_SIZE(tc); i++)
		free(tc[i].value);

	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.setup = setup,
	.test = verify_fgetxattr,
	.cleanup = cleanup,
	.tcnt = ARRAY_SIZE(tc),
	.mntpoint = MNTPOINT,
	.mount_device = 1,
	.all_filesystems = 1,
	.needs_root = 1,
};

#else /* HAVE_SYS_XATTR_H */
TST_TEST_TCONF("<sys/xattr.h> does not exist");
#endif
