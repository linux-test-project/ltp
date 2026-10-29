// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 RT-RK Institute for Computer Based Systems
 * Copyright (c) Linux Test Project, 2016-2024
 * Author: Dejan Jovicevic <dejan.jovicevic@rt-rk.com>
 */

/*\
 * Test for listxattr error.
 *
 * - ERANGE - the size of the list buffer is too small to hold the result.
 * - ENOENT - path is an empty string.
 * - EFAULT - attempted to read from a invalid address.
 * - ENAMETOOLONG - path is longer than allowed.
 */

#include "config.h"
#include <errno.h>
#include <sys/types.h>

#ifdef HAVE_SYS_XATTR_H
# include <sys/xattr.h>
#endif

#include "tst_test.h"

#ifdef HAVE_SYS_XATTR_H

#define SECURITY_KEY	"security.ltptest"
#define VALUE	"test"
#define VALUE_SIZE	(sizeof(VALUE) - 1)
#define TESTFILE    "testfile"

static char longpathname[PATH_MAX + 2];

static struct test_case {
	const char *path;
	size_t size;
	int exp_err;
} tc[] = {
	{TESTFILE, 1, ERANGE},
	{"", 20, ENOENT},
	{(char *)-1, 20, EFAULT},
	{longpathname, 20, ENAMETOOLONG}
};

static void verify_listxattr(unsigned int n)
{
	struct test_case *t = tc + n;
	char buf[t->size];

	TEST(listxattr(t->path, buf, sizeof(buf)));
	if (TST_RET != -1) {
		tst_res(TFAIL,
			"listxattr() succeeded unexpectedly (returned %ld)",
			TST_RET);
		return;
	}

	if (t->exp_err != TST_ERR) {
		tst_res(TFAIL | TTERRNO, "listxattr() failed "
			 "unexpectedlly, expected %s",
			 tst_strerrno(t->exp_err));
	} else {
		tst_res(TPASS | TTERRNO,
			 "listxattr() failed as expected");
	}
}

static void setup(void)
{
	SAFE_TOUCH(TESTFILE, 0644, NULL);

	SAFE_SETXATTR(TESTFILE, SECURITY_KEY, VALUE, VALUE_SIZE, XATTR_CREATE);

	memset(&longpathname, 'a', sizeof(longpathname) - 1);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.needs_root = 1,
	.test = verify_listxattr,
	.tcnt = ARRAY_SIZE(tc),
	.setup = setup,
};

#else /* HAVE_SYS_XATTR_H */
	TST_TEST_TCONF("<sys/xattr.h> does not exist.");
#endif
