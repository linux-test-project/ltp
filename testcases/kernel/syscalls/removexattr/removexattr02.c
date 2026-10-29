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
* Test Name: removexattr02
*
* Description:
* 1) removexattr(2) fails if the named attribute does not exist.
* 2) removexattr(2) fails if path is an empty string.
* 3) removexattr(2) fails when attempted to read from a invalid address.
*
* Expected Result:
* 1) removexattr(2) should return -1 and set errno to ENODATA.
* 2) removcxattr(2) should return -1 and set errno to ENOENT.
* 3) removexattr(2) should return -1 and set errno to EFAULT.
*/

#include "config.h"
#include <errno.h>
#include <sys/types.h>

#ifdef HAVE_SYS_XATTR_H
# include <sys/xattr.h>
#endif

#include "test.h"
#include "tso_safe_macros.h"

char *TCID = "removexattr02";

#ifdef HAVE_SYS_XATTR_H

static struct test_case {
	const char *path;
	char *name;
	int exp_err;
} tc[] = {
	/* test1 */
	{"testfile", "user.test", ENODATA},
	/* test2 */
	{"", "user.test", ENOENT},
	/* test3 */
	{(char *)-1, "user.test", EFAULT}
};

static void verify_removexattr(struct test_case *tc);
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
			verify_removexattr(&tc[i]);
	}

	cleanup();
	tst_exit();
}

static void verify_removexattr(struct test_case *tc)
{

	TEST(removexattr(tc->path, tc->name));
	if (TEST_RETURN == -1 && TEST_ERRNO == ENOTSUP) {
		tst_brkm(TCONF, cleanup, "No xattr support in fs or "
			 "mount without user_xattr option");
	}

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "removexattr() succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO != tc->exp_err) {
		tst_resm(TFAIL | TTERRNO, "removexattr() failed unexpectedly,"
			 " expected %s", tst_strerrno(tc->exp_err));
	} else {
		tst_resm(TPASS | TTERRNO,
			 "removexattr() failed as expected");
	}
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	SAFE_TOUCH(cleanup, "testfile", 0644, NULL);
}

static void cleanup(void)
{
	tst_rmdir();
}

#else /* HAVE_SYS_XATTR_H */
int main(int ac, char **av)
{
	tst_brkm(TCONF, NULL, "<sys/xattr.h> does not exist.");
}
#endif
