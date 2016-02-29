/*
* Copyright (c) 2016 Fujitsu Ltd.
* Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
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
*/

/*
* Test Name: llistxattr02
*
* Description:
* 1) llistxattr(2) fails if the size of the list buffer is too small
* to hold the result.
* 2) llistxattr(2) fails if path is an empty string.
* 3) llistxattr(2) fails when attempted to read from a invalid address.
*
* Expected Result:
* 1) llistxattr(2) should return -1 and set errno to ERANGE.
* 2) llistxattr(2) should return -1 and set errno to ENOENT.
* 3) llistxattr(2) should return -1 and set errno to EFAULT.
*/

#include "config.h"
#include <errno.h>
#include <sys/types.h>

#ifdef HAVE_ATTR_XATTR_H
# include <attr/xattr.h>
#endif

#include "tst_test.h"

#ifdef HAVE_ATTR_XATTR_H

#define SECURITY_KEY	"security.ltptest"
#define VALUE	"test"
#define VALUE_SIZE	4

static struct test_case {
	const char *path;
	size_t size;
	int exp_err;
} tc[] = {
	/* test1 */
	{"symlink", 1, ERANGE},
	/* test2 */
	{"", 20, ENOENT},
	/* test3 */
	{(char *)-1, 20, EFAULT}
};

static void verify_llistxattr(unsigned int n)
{
	struct test_case *t = tc + n;
	char buf[t->size];

	TEST(llistxattr(t->path, buf, t->size));
	if (TEST_RETURN != -1) {
		tst_res(TFAIL, "llistxattr() succeeded unexpectedly");
	} else {
		if (TEST_ERRNO != t->exp_err) {
			tst_res(TFAIL | TTERRNO, "llistxattr() failed "
				 "unexpectedlly, expected %s",
				 tst_strerrno(t->exp_err));
		} else {
			tst_res(TPASS | TTERRNO,
				 "llistxattr() failed as expected");
		}
	}
}

static void setup(void)
{
	int n;

	SAFE_TOUCH("testfile", 0644, NULL);

	SAFE_SYMLINK("testfile", "symlink");

	n = lsetxattr("symlink", SECURITY_KEY, VALUE, VALUE_SIZE, XATTR_CREATE);
	if (n == -1) {
		if (errno == ENOTSUP) {
			tst_brk(TCONF, "no xattr support in fs or "
				 "mounted without user_xattr option");
		} else {
			tst_brk(TBROK | TERRNO, "lsetxattr() failed");
		}
	}
}

static struct tst_test test = {
	.tid = "llistxattr02",
	.needs_tmpdir = 1,
	.needs_root = 1,
	.test = verify_llistxattr,
	.tcnt = ARRAY_SIZE(tc),
	.setup = setup,
};

#else /* HAVE_ATTR_XATTR_H */
	TST_TEST_TCONF("<attr/xattr.h> does not exist.");
#endif
