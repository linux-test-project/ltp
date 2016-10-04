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
#include "test.h"

char *TCID = "setxattr01";

#ifdef HAVE_SYS_XATTR_H
#define XATTR_NAME_MAX 255
#define XATTR_NAME_LEN (XATTR_NAME_MAX + 2)
#define XATTR_SIZE_MAX 65536
#define XATTR_TEST_KEY "user.testkey"
#define XATTR_TEST_VALUE "this is a test value"
#define XATTR_TEST_VALUE_SIZE 20

static void setup(void);
static void cleanup(void);

char filename[BUFSIZ];
char long_key[XATTR_NAME_LEN];
char *long_value;

struct test_case {
	char *fname;
	char *key;
	char *value;
	size_t size;
	int flags;
	int exp_err;
};
struct test_case tc[] = {
	{			/* case 00, invalid flags */
	 .fname = filename,
	 .key = XATTR_TEST_KEY,
	 .value = XATTR_TEST_VALUE,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = ~0,
	 .exp_err = EINVAL,
	 },
	{			/* case 01, replace non-existing attribute */
	 .fname = filename,
	 .key = XATTR_TEST_KEY,
	 .value = XATTR_TEST_VALUE,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = XATTR_REPLACE,
	 .exp_err = ENODATA,
	 },
	{			/* case 02, long key name, key will be set in setup() */
	 .fname = filename,
	 .key = NULL,
	 .value = XATTR_TEST_VALUE,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = XATTR_CREATE,
	 .exp_err = ERANGE,
	 },
	{			/* case 03, long value, value will be set in setup() */
	 .fname = filename,
	 .key = XATTR_TEST_KEY,
	 .value = NULL,
	 .size = XATTR_SIZE_MAX + 1,
	 .flags = XATTR_CREATE,
	 .exp_err = E2BIG,
	 },
	{			/* case 04, zero length value */
	 .fname = filename,
	 .key = XATTR_TEST_KEY,
	 .value = XATTR_TEST_VALUE,
	 .size = 0,
	 .flags = XATTR_CREATE,
	 .exp_err = 0,
	 },
	{			/* case 05, create existing attribute */
	 .fname = filename,
	 .key = XATTR_TEST_KEY,
	 .value = XATTR_TEST_VALUE,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = XATTR_CREATE,
	 .exp_err = EEXIST,
	 },
	{			/* case 06, replace existing attribute */
	 .fname = filename,
	 .key = XATTR_TEST_KEY,
	 .value = XATTR_TEST_VALUE,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = XATTR_REPLACE,
	 .exp_err = 0,
	 },
};

int TST_TOTAL = sizeof(tc) / sizeof(tc[0]);

int main(int argc, char *argv[])
{
	int lc;
	int i;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			TEST(setxattr(tc[i].fname, tc[i].key, tc[i].value,
				      tc[i].size, tc[i].flags));

			if (TEST_ERRNO == tc[i].exp_err) {
				tst_resm(TPASS | TTERRNO, "expected behavior");
			} else {
				tst_resm(TFAIL | TTERRNO, "unexpected behavior "
					 "- expected errno %d - Got",
					 tc[i].exp_err);
			}
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	int fd;

	tst_require_root();

	tst_tmpdir();

	/* Test for xattr support */
	fd = creat("testfile", 0644);
	if (fd == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Create testfile failed");
	close(fd);
	if (setxattr("testfile", "user.test", "test", 4, XATTR_CREATE) == -1)
		if (errno == ENOTSUP)
			tst_brkm(TCONF, cleanup, "No xattr support in fs or "
				 "mount without user_xattr option");

	/* Create test file */
	snprintf(filename, BUFSIZ, "setxattr01testfile");
	fd = creat(filename, 0644);
	if (fd == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Create test file(%s) failed",
			 filename);
	close(fd);

	/* Prepare test cases */
	snprintf(long_key, 6, "%s", "user.");
	memset(long_key + 5, 'k', XATTR_NAME_LEN - 5);
	long_key[XATTR_NAME_LEN - 1] = '\0';
	tc[2].key = long_key;

	long_value = malloc(XATTR_SIZE_MAX + 2);
	if (!long_value)
		tst_brkm(TBROK | TERRNO, cleanup, "malloc failed");
	memset(long_value, 'v', XATTR_SIZE_MAX + 2);
	long_value[XATTR_SIZE_MAX + 1] = '\0';
	tc[3].value = long_value;

	TEST_PAUSE;
}

static void cleanup(void)
{
	tst_rmdir();
}
#else /* HAVE_SYS_XATTR_H */
int main(int argc, char *argv[])
{
	tst_brkm(TCONF, NULL, "<sys/xattr.h> does not exist.");
}
#endif
