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
* Test Name: llistxattr01
*
* Description:
* The testcase checks the basic functionality of the llistxattr(2).
* llistxattr(2) retrieves the list of extended attribute names
* associated with the link itself in the filesystem.
*
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

#define SECURITY_KEY1   "security.ltptest1"
#define SECURITY_KEY2   "security.ltptest2"
#define VALUE           "test"
#define VALUE_SIZE      (sizeof(VALUE) - 1)
#define KEY_SIZE        (sizeof(SECURITY_KEY1) - 1)
#define TESTFILE        "testfile"
#define SYMLINK         "symlink"


static int has_attribute(const char *list, int llen, const char *attr)
{
	int i;

	for (i = 0; i < llen; i += strlen(list + i) + 1) {
		if (!strcmp(list + i, attr))
			return 1;
	}
	return 0;
}

static void verify_llistxattr(void)
{
	char buf[64];

	TEST(llistxattr(SYMLINK, buf, sizeof(buf)));
	if (TEST_RETURN == -1) {
		tst_res(TFAIL | TTERRNO, "llistxattr() failed");
		return;
	}

	if (has_attribute(buf, sizeof(buf), SECURITY_KEY1)) {
		tst_res(TFAIL, "get file attribute %s unexpectlly",
			 SECURITY_KEY1);
		return;
	}

	if (!has_attribute(buf, sizeof(buf), SECURITY_KEY2)) {
		tst_res(TFAIL, "missing attribute %s", SECURITY_KEY2);
		return;
	}

	tst_res(TPASS, "llistxattr() succeeded");
}

static void setup(void)
{
	SAFE_TOUCH(TESTFILE, 0644, NULL);

	SAFE_SYMLINK(TESTFILE, SYMLINK);

	SAFE_LSETXATTR(TESTFILE, SECURITY_KEY1, VALUE, VALUE_SIZE, XATTR_CREATE);

	SAFE_LSETXATTR(SYMLINK, SECURITY_KEY2, VALUE, VALUE_SIZE, XATTR_CREATE);
}

static struct tst_test test = {
	.tid = "llistxattr01",
	.needs_tmpdir = 1,
	.needs_root = 1,
	.test_all = verify_llistxattr,
	.setup = setup,
};

#else
	TST_TEST_TCONF("<sys/xattr.h> does not exist.");
#endif /* HAVE_SYS_XATTR_H */

