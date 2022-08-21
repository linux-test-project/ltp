// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Internstional Business Machines  Corp., 2006
 * Author: Yi Yang <yyangcdl@cn.ibm.com>
 * Copyright (c) Cyril Hrubis 2014 <chrubis@suse.cz>
 */

/*\
 * [Description]
 *
 * This test case will verify basic function of openat.
 *
 * - pathname is relative, then it is interpreted relative to the directory
 *   referred to by the file descriptor dirfd
 *
 * - pathname is absolute, then dirfd is ignored
 *
 * - ENODIR pathname is a relative pathname and dirfd is a file
 *   descriptor referring to a file other than a directory
 *
 * - EBADF dirfd is not a valid file descriptor
 *
 * - pathname is relative and dirfd is the special value AT_FDCWD, then pathname
 *   is interpreted relative to the current working directory of the calling
 *   process
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "tst_test.h"

#define TEST_FILE "test_file"
#define TEST_DIR "test_dir/"

static int dir_fd, fd;
static int fd_invalid = 100;
static int fd_atcwd = AT_FDCWD;
static char glob_path[256];

static struct test_case {
	int *dir_fd;
	const char *pathname;
	int exp_ret;
	int exp_errno;
} test_cases[] = {
	{&dir_fd, TEST_FILE, 0, 0},
	{&dir_fd, glob_path, 0, 0},
	{&fd, TEST_FILE, -1, ENOTDIR},
	{&fd_invalid, TEST_FILE, -1, EBADF},
	{&fd_atcwd, TEST_DIR TEST_FILE, 0, 0}
};

static void verify_openat(unsigned int n)
{
	struct test_case *tc = &test_cases[n];

	if (tc->exp_ret) {
		if (tc->exp_errno == ENOTDIR) {
			TST_EXP_FAIL2(openat(*tc->dir_fd, tc->pathname, O_RDWR, 0600),
				ENOTDIR, "openat with a filefd instead of dirfd");
		} else {
			TST_EXP_FAIL2(openat(*tc->dir_fd, tc->pathname, O_RDWR, 0600),
				EBADF, "openat with invalid fd");
		}
	} else {
		TST_EXP_FD(openat(*tc->dir_fd, tc->pathname, O_RDWR, 0600));
	}

	if (TST_RET > 0)
		SAFE_CLOSE(TST_RET);
}

static void setup(void)
{
	char buf[PATH_MAX];

	SAFE_GETCWD(buf, PATH_MAX);
	SAFE_MKDIR(TEST_DIR, 0700);
	dir_fd = SAFE_OPEN(TEST_DIR, O_DIRECTORY);
	fd = SAFE_OPEN(TEST_DIR TEST_FILE, O_CREAT | O_RDWR, 0600);

	snprintf(glob_path, sizeof(glob_path), "%s/" TEST_DIR TEST_FILE, buf);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
	if (dir_fd > 0)
		SAFE_CLOSE(dir_fd);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_openat,
	.tcnt = ARRAY_SIZE(test_cases),
	.needs_tmpdir = 1,
};
