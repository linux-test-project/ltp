/*
*  Copyright (c) 2016 RT-RK Institute for Computer Based Systems
*  Author: Dejan Jovicevic <dejan.jovicevic@rt-rk.com>
*
*  This program is free software;  you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY;  without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
*  the GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.
*/

/*
* Test Name: listxattr01
*
* Description:
* The testcase checks the basic functionality of the listxattr(2).
* listxattr(2) retrieves the list of extended attribute names
* associated with the file itself in the filesystem.
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

#define SECURITY_KEY1	"security.ltptest1"
#define VALUE	"test"
#define VALUE_SIZE	(sizeof(VALUE) - 1)
#define KEY_SIZE    (sizeof(SECURITY_KEY1) - 1)
#define TESTFILE    "testfile"

static int has_attribute(const char *list, int llen, const char *attr)
{
	int i;

	for (i = 0; i < llen; i += strlen(list + i) + 1) {
		if (!strcmp(list + i, attr))
			return 1;
	}
	return 0;
}

static void verify_listxattr(void)
{
	char buf[64];

	TEST(listxattr(TESTFILE, buf, sizeof(buf)));
	if (TEST_RETURN == -1) {
		tst_res(TFAIL | TTERRNO, "listxattr() failed");
		return;
	}

	if (!has_attribute(buf, sizeof(buf), SECURITY_KEY1)) {
		tst_res(TFAIL, "missing attribute %s",
			 SECURITY_KEY1);
		return;
	}

	tst_res(TPASS, "listxattr() succeeded");
}

static void setup(void)
{
	SAFE_TOUCH(TESTFILE, 0644, NULL);

	SAFE_SETXATTR(TESTFILE, SECURITY_KEY1, VALUE, VALUE_SIZE, XATTR_CREATE);
}

static struct tst_test test = {
	.tid = "listxattr01",
	.needs_tmpdir = 1,
	.needs_root = 1,
	.test_all = verify_listxattr,
	.setup = setup,
};

#else
	TST_TEST_TCONF("<sys/xattr.h> does not exist.");
#endif /* HAVE_SYS_XATTR_H */
