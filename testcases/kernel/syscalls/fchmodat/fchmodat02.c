// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2006
 * Copyright (c) Linux Test Project, 2003-2023
 * Author: Yi Yang <yyangcdl@cn.ibm.com>
 */

/*\
 * [Description]
 *
 * - fchmodat() fails with ENOTDIR if dir_fd is file descriptor
 *   to the file and pathname is relative path of the file.
 * - fchmodat() fails with EBADF if dir_fd is invalid.
 * - fchmodat() fails with EFAULT if pathname points outside
 *   the accessible address space.
 * - fchmodat() fails with ENAMETOOLONG if path is too long.
 * - fchmodat() fails with ENOENT if pathname does not exist.
 * - fchmodat() fails with EINVAL if flag is invalid.
 */

#include <stdlib.h>
#include <stdio.h>
#include "tst_test.h"

#define TESTFILE        "fchmodatfile"

static int file_fd;
static int bad_fd = -1;
static char path[PATH_MAX + 2];
static char *long_path = path;
static int fd_atcwd = AT_FDCWD;
static char *bad_path;
static char *test_path;
static char *empty_path;

static struct tcase {
	int *fd;
	char **filenames;
	int flag;
	int exp_errno;
	const char *desc;
} tcases[] = {
	{&file_fd, &test_path, 0, ENOTDIR, "fd pointing to file"},
	{&bad_fd, &test_path, 0, EBADF, "invalid fd"},
	{&file_fd, &bad_path, 0, EFAULT, "invalid address"},
	{&file_fd, &long_path, 0, ENAMETOOLONG, "pathname too long"},
	{&file_fd, &empty_path, 0, ENOENT, "path is empty"},
	{&fd_atcwd, &test_path, -1, EINVAL, "invalid flag"},
};

static void verify_fchmodat(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	TST_EXP_FAIL(fchmodat(*tc->fd, *tc->filenames, 0600, tc->flag),
		     tc->exp_errno, "fchmodat() with %s", tc->desc);
}

static void setup(void)
{
	file_fd = SAFE_OPEN(TESTFILE, O_CREAT | O_RDWR, 0600);

	bad_path = tst_get_bad_addr(NULL);

	memset(path, 'a', PATH_MAX + 2);
}

static void cleanup(void)
{
	if (file_fd > -1)
		SAFE_CLOSE(file_fd);
}

static struct tst_test test = {
	.test = verify_fchmodat,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.bufs = (struct tst_buffers []) {
		{&test_path, .str = TESTFILE},
		{&empty_path, .str = ""},
		{},
	},
	.needs_tmpdir = 1,
};
