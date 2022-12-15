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
 * have extended attributes. Otherwise getxattr(2) will return -1
 * and set errno to ENODATA.
 *
 * There are 4 test cases:
 * 1. Get attribute from a FIFO, setxattr(2) should return -1 and
 *    set errno to ENODATA
 * 2. Get attribute from a char special file, setxattr(2) should
 *    return -1 and set errno to ENODATA
 * 3. Get attribute from a block special file, setxattr(2) should
 *    return -1 and set errno to ENODATA
 * 4. Get attribute from a UNIX domain socket, setxattr(2) should
 *    return -1 and set errno to ENODATA
 */

#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
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
#include "safe_macros.h"

char *TCID = "getxattr02";

#ifdef HAVE_SYS_XATTR_H
#define XATTR_TEST_KEY "user.testkey"

#define FIFO "getxattr02fifo"
#define CHR  "getxattr02chr"
#define BLK  "getxattr02blk"
#define SOCK "getxattr02sock"

static void setup(void);
static void cleanup(void);

static char *tc[] = {
	FIFO,			/* case 00, get attr from fifo */
	CHR,			/* case 01, get attr from char special */
	BLK,			/* case 02, get attr from block special */
	SOCK,			/* case 03, get attr from UNIX domain socket */
};

int TST_TOTAL = sizeof(tc) / sizeof(tc[0]);

int main(int argc, char *argv[])
{
	int lc;
	int i;
	int exp_eno;
	char buf[BUFSIZ];

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		exp_eno = ENODATA;

		for (i = 0; i < TST_TOTAL; i++) {
			TEST(getxattr(tc[0], XATTR_TEST_KEY, buf, BUFSIZ));

			if (TEST_RETURN == -1 && TEST_ERRNO == exp_eno)
				tst_resm(TPASS | TTERRNO, "expected behavior");
			else
				tst_resm(TFAIL | TTERRNO, "unexpected behavior"
					 " - expected errno %d - Got", exp_eno);
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
	fd = SAFE_CREAT(cleanup, "testfile", 0644);
	close(fd);
	if (setxattr("testfile", "user.test", "test", 4, XATTR_CREATE) == -1)
		if (errno == ENOTSUP)
			tst_brkm(TCONF, cleanup, "No xattr support in fs or "
				 "mount without user_xattr option");
	unlink("testfile");

	/* Create test files */
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
