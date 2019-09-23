/*
 * Copyright (C) 2011  Red Hat, Inc.
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
 * Basic tests for getxattr(2) and make sure getxattr(2) handles error
 * conditions correctly.
 *
 * There are 4 test cases:
 * 1. Get an non-existing attribute,
 *    getxattr(2) should return -1 and set errno to ENODATA
 * 2. Buffer size is smaller than attribute value size,
 *    getxattr(2) should return -1 and set errno to ERANGE
 * 3. Get attribute, getxattr(2) should succeed
 * 4. Verify the attribute got by getxattr(2) is same as the value we set
 */

#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_SYS_XATTR_H
# include <sys/xattr.h>
#endif
#include "test.h"
#include "safe_macros.h"

char *TCID = "getxattr01";

#ifdef HAVE_SYS_XATTR_H
#define XATTR_TEST_KEY "user.testkey"
#define XATTR_TEST_VALUE "this is a test value"
#define XATTR_TEST_VALUE_SIZE 20
#define BUFFSIZE 64

static void setup(void);
static void cleanup(void);

char filename[BUFSIZ];

struct test_case {
	char *fname;
	char *key;
	char *value;
	size_t size;
	int exp_err;
};
struct test_case tc[] = {
	{			/* case 00, get non-existing attribute */
	 .fname = filename,
	 .key = "user.nosuchkey",
	 .value = NULL,
	 .size = BUFFSIZE - 1,
	 .exp_err = ENODATA,
	 },
	{			/* case 01, small value buffer */
	 .fname = filename,
	 .key = XATTR_TEST_KEY,
	 .value = NULL,
	 .size = 1,
	 .exp_err = ERANGE,
	 },
	{			/* case 02, get existing attribute */
	 .fname = filename,
	 .key = XATTR_TEST_KEY,
	 .value = NULL,
	 .size = BUFFSIZE - 1,
	 .exp_err = 0,
	 },
};

int TST_TOTAL = sizeof(tc) / sizeof(tc[0]) + 1;

int main(int argc, char *argv[])
{
	int lc;
	int i;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < ARRAY_SIZE(tc); i++) {
			TEST(getxattr(tc[i].fname, tc[i].key, tc[i].value,
				      tc[i].size));

			if (TEST_ERRNO == tc[i].exp_err) {
				tst_resm(TPASS | TTERRNO, "expected behavior");
			} else {
				tst_resm(TFAIL | TTERRNO, "unexpected behavior"
					 "- expected errno %d - Got",
					 tc[i].exp_err);
			}
		}

		if (TEST_RETURN != XATTR_TEST_VALUE_SIZE) {
			tst_resm(TFAIL,
				 "getxattr() returned wrong size %ld expected %d",
				 TEST_RETURN, XATTR_TEST_VALUE_SIZE);
			continue;
		}

		if (memcmp(tc[i - 1].value, XATTR_TEST_VALUE, XATTR_TEST_VALUE_SIZE))
			tst_resm(TFAIL, "Wrong value, expect \"%s\" got \"%s\"",
				 XATTR_TEST_VALUE, tc[i - 1].value);
		else
			tst_resm(TPASS, "Got the right value");
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	int fd;
	int i;

	tst_require_root();

	tst_tmpdir();

	/* Create test file and setup initial xattr */
	snprintf(filename, BUFSIZ, "getxattr01testfile");
	fd = SAFE_CREAT(cleanup, filename, 0644);
	close(fd);
	if (setxattr(filename, XATTR_TEST_KEY, XATTR_TEST_VALUE,
		     strlen(XATTR_TEST_VALUE), XATTR_CREATE) == -1) {
		if (errno == ENOTSUP) {
			tst_brkm(TCONF, cleanup, "No xattr support in fs or "
				 "mount without user_xattr option");
		}
	}

	/* Prepare test cases */
	for (i = 0; i <  ARRAY_SIZE(tc); i++) {
		tc[i].value = malloc(BUFFSIZE);
		if (tc[i].value == NULL) {
			tst_brkm(TBROK | TERRNO, cleanup,
				 "Cannot allocate memory");
		}
	}

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
