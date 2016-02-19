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

#ifdef HAVE_ATTR_XATTR_H
#include <attr/xattr.h>
#endif

#include "test.h"
#include "safe_macros.h"
#include "safe_file_ops.h"

char *TCID = "llistxattr01";

#ifdef HAVE_ATTR_XATTR_H
#define SECURITY_KEY1	"security.ltptest1"
#define SECURITY_KEY2	"security.ltptest2"
#define VALUE	"test"
#define VALUE_SIZE	4
#define KEY_SIZE    17

static void verify_llistxattr(void);
static void setup(void);
static void set_xattr(const char *, const char *);
static int has_attribute(const char *, int, const char *);
static void cleanup(void);

int TST_TOTAL = 1;

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		verify_llistxattr();
	}

	cleanup();
	tst_exit();
}

static void verify_llistxattr(void)
{
	int size = 64;
	char buf[size];

	TEST(llistxattr("symlink", buf, size));
	if (TEST_RETURN == -1) {
		tst_resm(TFAIL | TERRNO, "llistxattr() failed");
		return;
	}

	if (has_attribute(buf, size, SECURITY_KEY1)) {
		tst_resm(TFAIL, "get file attribute %s unexpectlly",
			 SECURITY_KEY1);
		return;
	}

	if (!has_attribute(buf, size, SECURITY_KEY2)) {
		tst_resm(TFAIL, "missing attribute %s", SECURITY_KEY2);
		return;
	}

	tst_resm(TPASS, "llistxattr() succeeded");
}

static void setup(void)
{
	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	SAFE_TOUCH(cleanup, "testfile", 0644, NULL);

	SAFE_SYMLINK(cleanup, "testfile", "symlink");

	set_xattr("testfile", SECURITY_KEY1);

	set_xattr("symlink", SECURITY_KEY2);
}

static void set_xattr(const char *path, const char *key)
{
	int n;

	n = lsetxattr(path, key, VALUE, VALUE_SIZE, XATTR_CREATE);
	if (n == -1) {
		if (errno == ENOTSUP) {
			tst_brkm(TCONF, cleanup,
				 "no xattr support in fs or mounted "
				 "without user_xattr option");
		}

		if (errno == EEXIST) {
			tst_brkm(TBROK, cleanup, "exist attribute %s", key);
		} else {
			tst_brkm(TBROK | TERRNO, cleanup,
				 "lsetxattr() failed");
		}
	}
}

static int has_attribute(const char *list, int llen, const char *attr)
{
	int i;

	for (i = 0; i < llen; i += strlen(list + i) + 1) {
		if (!strcmp(list + i, attr))
			return 1;
	}
	return 0;
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
