// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2006
 * Copyright (c) Linux Test Project, 2003-2023
 * Author: Yi Yang <yyangcdl@cn.ibm.com>
 */

/*\
 * - faccessat() fails with ENOTDIR if dir_fd is file descriptor to the file
 *   and pathname is relative path of the file.
 *
 * - faccessat() fails with EBADF if dir_fd is invalid.
 */

#include <stdlib.h>
#include <stdio.h>
#include "tst_test.h"

#define TESTDIR         "faccessatdir"
#define TESTFILE        "faccessatfile"
#define FILEPATH        "faccessatdir/faccessatfile"

static int dir_fd, file_fd;
static int bad_fd = -1;

static struct tcase {
	int *fd;
	int exp_errno;
} tcases[] = {
	{&file_fd, ENOTDIR},
	{&bad_fd, EBADF},
};

static void verify_faccessat(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	TST_EXP_FAIL(faccessat(*tc->fd, TESTFILE, R_OK, 0),
		     tc->exp_errno, "faccessat(%d, TESTFILE, R_OK, 0)",
		     *tc->fd);
}

static void setup(void)
{
	SAFE_MKDIR(TESTDIR, 0700);
	dir_fd = SAFE_OPEN(TESTDIR, O_DIRECTORY);
	file_fd = SAFE_OPEN(FILEPATH, O_CREAT | O_RDWR, 0600);
}

static void cleanup(void)
{
	if (dir_fd > -1)
		SAFE_CLOSE(dir_fd);

	if (file_fd > -1)
		SAFE_CLOSE(file_fd);
}

static struct tst_test test = {
	.test = verify_faccessat,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
