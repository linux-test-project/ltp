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
#include <attr/xattr.h>
#endif

#include "test.h"
#include "safe_macros.h"

char *TCID = "llistxattr02";

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

static void verify_llistxattr(struct test_case *tc);
static void setup(void);
static void cleanup(void);

int TST_TOTAL = ARRAY_SIZE(tc);

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++)
			verify_llistxattr(&tc[i]);
	}

	cleanup();
	tst_exit();
}

static void verify_llistxattr(struct test_case *tc)
{
	char buf[tc->size];

	TEST(llistxattr(tc->path, buf, tc->size));
	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "llistxattr() succeeded unexpectedly");
	} else {
		if (TEST_ERRNO != tc->exp_err) {
			tst_resm(TFAIL | TTERRNO, "llistxattr() failed "
				 "unexpectedlly, expected %s",
				 tst_strerrno(tc->exp_err));
		} else {
			tst_resm(TPASS | TTERRNO,
				 "llistxattr() failed as expected");
		}
	}
}

static void setup(void)
{
	int n;

	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	SAFE_TOUCH(cleanup, "testfile", 0644, NULL);

	SAFE_SYMLINK(cleanup, "testfile", "symlink");

	n = lsetxattr("symlink", SECURITY_KEY, VALUE, VALUE_SIZE, XATTR_CREATE);
	if (n == -1) {
		if (errno == ENOTSUP) {
			tst_brkm(TCONF, cleanup, "no xattr support in fs or "
				 "mounted without user_xattr option");
		} else {
			tst_brkm(TBROK | TERRNO, cleanup, "lsetxattr() failed");
		}
	}
}

static void cleanup(void)
{
	tst_rmdir();
}

#else /* HAVE_ATTR_XATTR_H */
int main(int ac, char **av)
{
	tst_brkm(TCONF, NULL, "<attr/xattr.h> does not exist.");
}
#endif
