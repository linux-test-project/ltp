/*
 * Copyright (C) 2011 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/*
 * Basic tests for setxattr(2) and make sure setxattr(2) handles error
 * conditions correctly.
 *
 * There are 7 test cases:
 * 1. Any other flags being set except XATTR_CREATE and XATTR_REPLACE,
 *    setxattr(2) should return -1 and set errno to EINVAL
 * 2. With XATTR_REPLACE flag set but the attribute does not exist,
 *    setxattr(2) should return -1 and set errno to ENODATA
 * 3. Create new attr with name length greater than XATTR_NAME_MAX(255)
 *    setxattr(2) should return -1 and set errno to ERANGE
 * 4. Create new attr whose value length is greater than XATTR_SIZE_MAX(65536)
 *    setxattr(2) should return -1 and set errno to E2BIG
 * 5. Create new attr whose value length is zero,
 *    setxattr(2) should succeed
 * 6. Replace the attr value without XATTR_REPLACE flag being set,
 *    setxattr(2) should return -1 and set errno to EEXIST
 * 7. Replace attr value with XATTR_REPLACE flag being set,
 *    setxattr(2) should succeed
 * 8. Create new attr whose key length is zero,
 *    setxattr(2) should return -1 and set errno to ERANGE
 * 9. Create new attr whose key is NULL,
 *    setxattr(2) should return -1 and set errno to EFAULT
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
#define XATTR_NAME_MAX 255
#define XATTR_NAME_LEN (XATTR_NAME_MAX + 2)
#define XATTR_SIZE_MAX 65536
#define XATTR_TEST_KEY "user.testkey"
#define XATTR_TEST_VALUE "this is a test value"
#define XATTR_TEST_VALUE_SIZE 20
#define MNTPOINT "mntpoint"
#define FNAME MNTPOINT"/setxattr01testfile"

static char long_key[XATTR_NAME_LEN];
static char *long_value;
static char *xattr_value = XATTR_TEST_VALUE;

struct test_case {
	char *key;
	char **value;
	size_t size;
	int flags;
	int exp_err;
	int keyneeded;
};
struct test_case tc[] = {
	{			/* case 00, invalid flags */
	 .key = XATTR_TEST_KEY,
	 .value = &xattr_value,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = ~0,
	 .exp_err = EINVAL,
	 },
	{			/* case 01, replace non-existing attribute */
	 .key = XATTR_TEST_KEY,
	 .value = &xattr_value,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = XATTR_REPLACE,
	 .exp_err = ENODATA,
	 },
	{			/* case 02, long key name */
	 .key = long_key,
	 .value = &xattr_value,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = XATTR_CREATE,
	 .exp_err = ERANGE,
	 },
	{			/* case 03, long value */
	 .key = XATTR_TEST_KEY,
	 .value = &long_value,
	 .size = XATTR_SIZE_MAX + 1,
	 .flags = XATTR_CREATE,
	 .exp_err = E2BIG,
	 },
	{			/* case 04, zero length value */
	 .key = XATTR_TEST_KEY,
	 .value = &xattr_value,
	 .size = 0,
	 .flags = XATTR_CREATE,
	 .exp_err = 0,
	 },
	{			/* case 05, create existing attribute */
	 .key = XATTR_TEST_KEY,
	 .value = &xattr_value,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = XATTR_CREATE,
	 .exp_err = EEXIST,
	 .keyneeded = 1,
	 },
	{			/* case 06, replace existing attribute */
	 .key = XATTR_TEST_KEY,
	 .value = &xattr_value,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = XATTR_REPLACE,
	 .exp_err = 0,
	 .keyneeded = 1,
	},
	{			/* case 07, zero length key */
	 .key = "",
	 .value = &xattr_value,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = XATTR_CREATE,
	 .exp_err = ERANGE,
	},
	{			/* case 08, NULL key */
	 .value = &xattr_value,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = XATTR_CREATE,
	 .exp_err = EFAULT,
	},
};

static void verify_setxattr(unsigned int i)
{
	/* some tests might require existing keys for each iteration */
	if (tc[i].keyneeded) {
		SAFE_SETXATTR(FNAME, tc[i].key, tc[i].value, tc[i].size,
				XATTR_CREATE);
	}

	TEST(setxattr(FNAME, tc[i].key, *tc[i].value, tc[i].size, tc[i].flags));

	if (TST_RET == -1 && TST_ERR == EOPNOTSUPP)
		tst_brk(TCONF, "setxattr(2) not supported");

	/* success */

	if (!tc[i].exp_err) {
		if (TST_RET) {
			tst_res(TFAIL | TTERRNO,
				"setxattr(2) failed with %li", TST_RET);
			return;
		}

		/* this is needed for subsequent iterations */
		SAFE_REMOVEXATTR(FNAME, tc[i].key);

		tst_res(TPASS, "setxattr(2) passed");

		return;
	}

	if (TST_RET == 0) {
		tst_res(TFAIL, "setxattr(2) passed unexpectedly");
		return;
	}

	/* error */

	if (tc[i].exp_err != TST_ERR) {
		tst_res(TFAIL | TTERRNO, "setxattr(2) should fail with %s",
			tst_strerrno(tc[i].exp_err));
		return;
	}

	/* key might have been added AND test might have failed, remove it */
	if (tc[i].keyneeded)
		SAFE_REMOVEXATTR(FNAME, tc[i].key);

	tst_res(TPASS | TTERRNO, "setxattr(2) failed");
}

static void setup(void)
{
	size_t i = 0;

	snprintf(long_key, 6, "%s", "user.");
	memset(long_key + 5, 'k', XATTR_NAME_LEN - 5);
	long_key[XATTR_NAME_LEN - 1] = '\0';

	long_value = SAFE_MALLOC(XATTR_SIZE_MAX + 2);
	memset(long_value, 'v', XATTR_SIZE_MAX + 2);
	long_value[XATTR_SIZE_MAX + 1] = '\0';

	SAFE_TOUCH(FNAME, 0644, NULL);

	for (i = 0; i < ARRAY_SIZE(tc); i++) {
		if (!tc[i].key)
			tc[i].key = tst_get_bad_addr(NULL);
	}
}

static struct tst_test test = {
	.setup = setup,
	.test = verify_setxattr,
	.tcnt = ARRAY_SIZE(tc),
	.mntpoint = MNTPOINT,
	.mount_device = 1,
	.all_filesystems = 1,
	.needs_tmpdir = 1,
	.needs_root = 1,
};

#else /* HAVE_SYS_XATTR_H */
TST_TEST_TCONF("<sys/xattr.h> does not exist");
#endif
