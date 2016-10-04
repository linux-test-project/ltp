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
 * In the user.* namespace, only regular files and directories can
 * have extended attributes. Otherwise setxattr(2) will return -1
 * and set errno to EPERM.
 *
 * There are 7 test cases:
 * 1. Set attribute to a regular file, setxattr(2) should succeed
 * 2. Set attribute to a directory, setxattr(2) should succeed
 * 3. Set attribute to a symlink which points to the regular file,
 *    setxattr(2) should return -1 and set errno to EEXIST
 * 4. Set attribute to a FIFO, setxattr(2) should return -1 and set
 *    errno to EPERM
 * 5. Set attribute to a char special file, setxattr(2) should
 *    return -1 and set errno to EPERM
 * 6. Set attribute to a block special file, setxattr(2) should
 *    return -1 and set errno to EPERM
 * 7. Set attribute to a UNIX domain socket, setxattr(2) should
 *    return -1 and set errno to EPERM
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

char *TCID = "setxattr02";

#ifdef HAVE_SYS_XATTR_H
#define XATTR_TEST_KEY "user.testkey"
#define XATTR_TEST_VALUE "this is a test value"
#define XATTR_TEST_VALUE_SIZE 20

#define FILENAME "setxattr02testfile"
#define DIRNAME  "setxattr02testdir"
#define SYMLINK  "setxattr02symlink"
#define FIFO     "setxattr02fifo"
#define CHR      "setxattr02chr"
#define BLK      "setxattr02blk"
#define SOCK     "setxattr02sock"

static void setup(void);
static void cleanup(void);

struct test_case {
	char *fname;
	char *key;
	char *value;
	size_t size;
	int flags;
	int exp_err;
};
static struct test_case tc[] = {
	{			/* case 00, set attr to reg */
	 .fname = FILENAME,
	 .key = XATTR_TEST_KEY,
	 .value = XATTR_TEST_VALUE,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = XATTR_CREATE,
	 .exp_err = 0,
	 },
	{			/* case 01, set attr to dir */
	 .fname = DIRNAME,
	 .key = XATTR_TEST_KEY,
	 .value = XATTR_TEST_VALUE,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = XATTR_CREATE,
	 .exp_err = 0,
	 },
	{			/* case 02, set attr to symlink */
	 .fname = SYMLINK,
	 .key = XATTR_TEST_KEY,
	 .value = XATTR_TEST_VALUE,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = XATTR_CREATE,
	 .exp_err = EEXIST,
	 },
	{			/* case 03, set attr to fifo */
	 .fname = FIFO,
	 .key = XATTR_TEST_KEY,
	 .value = XATTR_TEST_VALUE,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = XATTR_CREATE,
	 .exp_err = EPERM,
	 },
	{			/* case 04, set attr to character special */
	 .fname = CHR,
	 .key = XATTR_TEST_KEY,
	 .value = XATTR_TEST_VALUE,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = XATTR_CREATE,
	 .exp_err = EPERM,
	 },
	{			/* case 05, set attr to block special */
	 .fname = BLK,
	 .key = XATTR_TEST_KEY,
	 .value = XATTR_TEST_VALUE,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = XATTR_CREATE,
	 .exp_err = EPERM,
	 },
	{			/* case 06, set attr to socket */
	 .fname = SOCK,
	 .key = XATTR_TEST_KEY,
	 .value = XATTR_TEST_VALUE,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = XATTR_CREATE,
	 .exp_err = EPERM,
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
	dev_t dev;

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
	unlink("testfile");

	/* Create test files */
	fd = creat(FILENAME, 0644);
	if (fd == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Create test file(%s) failed",
			 FILENAME);
	close(fd);

	if (mkdir(DIRNAME, 0644) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Create test dir(%s) failed",
			 DIRNAME);

	if (symlink(FILENAME, SYMLINK) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Create symlink(%s->%s)"
			 " failed", SYMLINK, FILENAME);

	if (mknod(FIFO, S_IFIFO | 0777, 0) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Create FIFO(%s) failed",
			 FIFO);

	dev = makedev(1, 3);
	if (mknod(CHR, S_IFCHR | 0777, dev) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Create char special(%s)"
			 " failed", CHR);

	if (mknod(BLK, S_IFBLK | 0777, 0) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Create block special(%s)"
			 " failed", BLK);

	if (mknod(SOCK, S_IFSOCK | 0777, 0) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Create socket(%s) failed",
			 SOCK);

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
