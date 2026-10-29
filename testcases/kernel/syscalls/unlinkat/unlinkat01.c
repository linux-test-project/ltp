// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2009-2021
 * Copyright (c) 2016 Oracle and/or its affiliates. All Rights Reserved.
 * Copyright (c) International Business Machines  Corp., 2006
 * Author: Yi Yang <yyangcdl@cn.ibm.com>
 */

/*\
 * Basic :manpage:`unlinkat(2)` test.
 */

#include "tst_test.h"
#include "lapi/syscalls.h"
#include "tst_safe_stdio.h"
#include "lapi/fcntl.h"

static const char pathname[] = "unlinkattestdir",
		  subpathname[] = "unlinkatsubtestdir",
		  subpathdir[] = "unlinkattestdir/unlinkatsubtestdir",
		  testfile[] = "unlinkattestfile.txt",
		  testfile2[] = "unlinkattestdir/unlinkattestfile.txt";

static char *testfile3;

static int fd;
static int getfd(int i)
{
	if (i == 2)
		fd = SAFE_OPEN(testfile3, O_CREAT | O_RDWR, 0600);
	else
		fd = SAFE_OPEN(pathname, O_DIRECTORY);

	return fd;
}

static struct tcase {
	int fd;
	const char *filename;
	int flag;
	int exp_errno;
} tc[] = {
	{0, testfile, 0, 0},
	{0, NULL, 0, 0},
	{0, testfile, 0, ENOTDIR},
	{100, testfile, 0, EBADF},
	{0, testfile, 9999, EINVAL},
	{AT_FDCWD, testfile, 0, 0},
	{0, subpathname, AT_REMOVEDIR, 0},
};

static void run(unsigned int i)
{
	int fd3 = -1;

	/* tesfile2 will be unlinked by test0. */
	if (access(testfile2, F_OK))
		SAFE_FILE_PRINTF(testfile2, testfile2);

	/* testfile3 will be unlined by test1. */
	if (access(testfile3, F_OK))
		fd3 = SAFE_OPEN(testfile3, O_CREAT | O_RDWR, 0600);

	/* subpathdir will be unlinked by test6. */
	if (access(subpathdir, F_OK))
		SAFE_MKDIR(subpathdir, 0700);

	/* testfile must exist except test1 and test6. */
	if (access(testfile, F_OK))
		SAFE_FILE_PRINTF(testfile, testfile);

	if (tc[i].fd)
		TEST(unlinkat(tc[i].fd, tc[i].filename, tc[i].flag));
	else
		TEST(unlinkat(getfd(i), tc[i].filename, tc[i].flag));

	if (TST_ERR == tc[i].exp_errno)
		tst_res(TPASS | TTERRNO, "unlinkat() returned expected errno");
	else
		tst_res(TFAIL | TTERRNO, "unlinkat() failed");

	if (!tc[i].fd)
		SAFE_CLOSE(fd);

	if (fd3 > 0)
		SAFE_CLOSE(fd3);
}

static void setup(void)
{
	char buf[PATH_MAX];
	SAFE_GETCWD(buf, PATH_MAX);
	SAFE_ASPRINTF(&testfile3, "%s/unlinkatfile3.txt", buf);
	tc[1].filename = testfile3;

	SAFE_MKDIR(pathname, 0700);
}

static void cleanup(void)
{
	SAFE_UNLINK(testfile);
	SAFE_UNLINK(testfile2);
	SAFE_RMDIR(pathname);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.tcnt = ARRAY_SIZE(tc),
	.setup = setup,
	.test = run,
	.cleanup = cleanup,
};
