// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Fujitsu Ltd.
 * Author: Jinbao Huang <huangjb.jy@cn.fujitsu.com>
 */

/*\
 * [Description]
 *
 * Verify that, lgetxattr(2) returns -1 and sets errno to
 *
 * 1. ENODATA if the named attribute does not exist.
 * 2. ERANGE if the size of the value buffer is too small to hold the result.
 * 3. EFAULT when reading from an invalid address.
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
	if (TST_RET != -1) {
		tst_res(TFAIL, "lgetxattr() succeeded unexpectedly");
		return;
	}

	if (TST_ERR != tc->exp_err) {
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
