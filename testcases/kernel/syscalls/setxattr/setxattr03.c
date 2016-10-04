/*
 * Copyright (C) 2012 Red Hat, Inc.
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
 * setxattr(2) to immutable and append-only files should get EPERM
 *
 * There are 2 test cases:
 * 1. Set attribute to a immutable file, setxattr(2) should return -1
 *    and set errno to EPERM
 * 2. Set attribute to a append-only file, setxattr(2) should return
 *    -1 and set errno to EPERM
 */

#include "config.h"
#include <sys/ioctl.h>
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
#include <linux/fs.h>

#include "test.h"

char *TCID = "setxattr03";

#if defined HAVE_SYS_XATTR_H && defined HAVE_FS_IOC_FLAGS
#define XATTR_TEST_KEY "user.testkey"
#define XATTR_TEST_VALUE "this is a test value"
#define XATTR_TEST_VALUE_SIZE (sizeof(XATTR_TEST_VALUE) - 1)

#define IMMU_FILE "setxattr03immutable"
#define APPEND_FILE  "setxattr03appendonly"

#define set_immutable_on(fd) fsetflag(fd, 1, 1)
#define set_immutable_off(fd) fsetflag(fd, 0, 1)
#define set_append_on(fd) fsetflag(fd, 1, 0)
#define set_append_off(fd) fsetflag(fd, 0, 0)

struct test_case {
	char *desc;
	char *fname;
	char *key;
	char *value;
	size_t size;
	int flags;
	int exp_err;
};
static struct test_case tc[] = {
	{			/* case 00, set attr to immutable file */
	 .desc = "Set attr to immutable file",
	 .fname = IMMU_FILE,
	 .key = XATTR_TEST_KEY,
	 .value = XATTR_TEST_VALUE,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = XATTR_CREATE,
	 .exp_err = EPERM,
	 },
	{			/* case 01, set attr to append-only file */
	 .desc = "Set attr to append-only file",
	 .fname = APPEND_FILE,
	 .key = XATTR_TEST_KEY,
	 .value = XATTR_TEST_VALUE,
	 .size = XATTR_TEST_VALUE_SIZE,
	 .flags = XATTR_CREATE,
	 .exp_err = EPERM,
	 },
};

static void setup(void);
static void cleanup(void);

static int immu_fd;
static int append_fd;

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
				tst_resm(TPASS | TTERRNO, "%s", tc[i].desc);
			} else {
				tst_resm(TFAIL | TTERRNO, "%s - expected errno"
					 " %d - Got", tc[i].desc,
					 tc[i].exp_err);
			}
		}
	}

	cleanup();
	tst_exit();
}

static int fsetflag(int fd, int on, int immutable)
{
	int fsflags = 0;
	int fsfl;

	if (ioctl(fd, FS_IOC_GETFLAGS, &fsflags) < 0)
		return 1;

	if (immutable)
		fsfl = FS_IMMUTABLE_FL;
	else
		fsfl = FS_APPEND_FL;

	if (on)
		fsflags |= fsfl;
	else
		fsflags &= ~fsfl;

	if (ioctl(fd, FS_IOC_SETFLAGS, &fsflags) < 0)
		return 1;

	return 0;
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
				 "fs mounted without user_xattr option");
	unlink("testfile");

	/* Create test files and set file immutable or append-only */
	immu_fd = creat(IMMU_FILE, 0644);
	if (immu_fd == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Create test file(%s) failed",
			 IMMU_FILE);
	if (set_immutable_on(immu_fd))
		tst_brkm(TBROK | TERRNO, cleanup, "Set %s immutable failed",
			 IMMU_FILE);

	append_fd = creat(APPEND_FILE, 0644);
	if (append_fd == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Create test file(%s) failed",
			 APPEND_FILE);
	if (set_append_on(append_fd))
		tst_brkm(TBROK | TERRNO, cleanup, "Set %s append-only failed",
			 APPEND_FILE);

	TEST_PAUSE;
}

static void cleanup(void)
{
	if ((immu_fd > 0) && set_immutable_off(immu_fd))
		tst_resm(TWARN | TERRNO, "Unset %s immutable failed",
			 IMMU_FILE);
	if ((append_fd > 0) && set_append_off(append_fd))
		tst_resm(TWARN | TERRNO, "Unset %s append-only failed",
			 APPEND_FILE);
	close(immu_fd);
	close(append_fd);

	tst_rmdir();
}
#else
int main(void)
{
	tst_brkm(TCONF, NULL, "<sys/xattr.h> not present or FS_IOC_FLAGS "
		 "missing in <linux/fs.h>");
}
#endif /* defined HAVE_SYS_XATTR_H && defined HAVE_FS_IOC_FLAGS */
