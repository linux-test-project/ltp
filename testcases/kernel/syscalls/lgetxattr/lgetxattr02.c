/*
* Copyright (c) 2016 Fujitsu Ltd.
* Author: Jinbao Huang <huangjb.jy@cn.fujitsu.com>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of version 2 of the GNU General Public License as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it would be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* You should have received a copy of the GNU General Public License
* alone with this program.
*
*/

/*
* Test Name: lgetxattr02
*
* Description:
* 1) lgetxattr(2) fails if the named attribute does not exist.
* 2) lgetxattr(2) fails if the size of the value buffer is too small
*    to hold the result.
* 3) lgetxattr(2) fails when attemptes to read from a invalid address.
*
* Expected Result:
* 1) lgetxattr(2) should return -1 and set errno to ENODATA.
* 2) lgetxattr(2) should return -1 and set errno to ERANGE.
* 3) lgetxattr(2) should return -1 and set errno to EFAULT.
*/

#include "config.h"
#include <errno.h>
#include <sys/types.h>
#include <string.h>

#ifdef HAVE_SYS_XATTR_H
# include <sys/xattr.h>
#endif

#include "tst_test.h"

#ifdef HAVE_SYS_XATTR_H

#define SECURITY_KEY	"security.ltptest"
#define VALUE	"this is a test value"

static struct test_case {
	const char *path;
	size_t size;
	int exp_err;
} tcase[] = {
	{"testfile", sizeof(VALUE), ENODATA},
	{"symlink", 1, ERANGE},
	{(char *)-1, sizeof(VALUE), EFAULT}
};

static void verify_lgetxattr(unsigned int n)
{
	struct test_case *tc = tcase + n;
	char buf[tc->size];

	TEST(lgetxattr(tc->path, SECURITY_KEY, buf, sizeof(buf)));
	if (TEST_RETURN != -1) {
		tst_res(TFAIL, "lgetxattr() succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO != tc->exp_err) {
		tst_res(TFAIL | TTERRNO, "lgetxattr() failed unexpectedlly, "
			"expected %s", tst_strerrno(tc->exp_err));
	} else {
		tst_res(TPASS | TTERRNO, "lgetxattr() failed as expected");
	}
}

static void setup(void)
{
	int res;

	SAFE_TOUCH("testfile", 0644, NULL);
	SAFE_SYMLINK("testfile", "symlink");

	res = lsetxattr("symlink", SECURITY_KEY, VALUE, strlen(VALUE), XATTR_CREATE);
	if (res == -1) {
		if (errno == ENOTSUP) {
			tst_brk(TCONF, "no xattr support in fs or "
				"mounted without user_xattr option");
		} else {
			tst_brk(TBROK | TERRNO, "lsetxattr(%s) failed",
				SECURITY_KEY);
		}
	}
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.needs_root = 1,
	.test = verify_lgetxattr,
	.tcnt = ARRAY_SIZE(tcase),
	.setup = setup
};

#else /* HAVE_SYS_XATTR_H */
	TST_TEST_TCONF("<sys/xattr.h> does not exist.");
#endif
