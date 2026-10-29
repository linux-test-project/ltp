// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2012 Red Hat, Inc.
 * Copyright (c) Linux Test Project, 2012-2024
 */

/*\
 * setxattr(2) to immutable and append-only files should get EPERM
 *
 * - Set attribute to a immutable file
 * - Set attribute to a append-only file
 */

#define _GNU_SOURCE
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
#include "lapi/fs.h"

#include "tst_test.h"

#if defined HAVE_SYS_XATTR_H
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

static int immu_fd;
static int append_fd;

static void verify_setxattr(unsigned int i)
{
	TEST(setxattr(tc[i].fname, tc[i].key, tc[i].value, tc[i].size,
			tc[i].flags));

	if (!TST_RET) {
		tst_res(TFAIL, "%s succeeded unexpectedly", tc[i].desc);
		return;
	}

	if (TST_ERR != tc[i].exp_err) {
		tst_res(TFAIL | TTERRNO, "%s - expected %s, got", tc[i].desc,
			tst_strerrno(tc[i].exp_err));
		return;
	}

	tst_res(TPASS | TTERRNO, "%s", tc[i].desc);
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

	/* Test for xattr support */
	fd = SAFE_CREAT("testfile", 0644);
	SAFE_CLOSE(fd);
	if (setxattr("testfile", "user.test", "test", 4, XATTR_CREATE) == -1)
		if (errno == ENOTSUP)
			tst_brk(TCONF, "No xattr support in fs or "
				 "fs mounted without user_xattr option");
	SAFE_UNLINK("testfile");

	/* Create test files and set file immutable or append-only */
	immu_fd = SAFE_CREAT(IMMU_FILE, 0644);
	if (set_immutable_on(immu_fd))
		tst_brk(TBROK | TERRNO, "Set %s immutable failed",
			 IMMU_FILE);

	append_fd = SAFE_CREAT(APPEND_FILE, 0644);
	if (set_append_on(append_fd))
		tst_brk(TBROK | TERRNO, "Set %s append-only failed",
			APPEND_FILE);
}

static void cleanup(void)
{
	if ((immu_fd > 0) && set_immutable_off(immu_fd))
		tst_res(TWARN | TERRNO, "Unset %s immutable failed",
			 IMMU_FILE);

	if ((append_fd > 0) && set_append_off(append_fd))
		tst_res(TWARN | TERRNO, "Unset %s append-only failed",
			 APPEND_FILE);

	if (immu_fd > 0)
		SAFE_CLOSE(immu_fd);

	if (append_fd > 0)
		SAFE_CLOSE(append_fd);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_setxattr,
	.tcnt = ARRAY_SIZE(tc),
	.needs_tmpdir = 1,
	.needs_root = 1,
};

#else
TST_TEST_TCONF("<sys/xattr.h> does not exist");
#endif /* defined HAVE_SYS_XATTR_H */
