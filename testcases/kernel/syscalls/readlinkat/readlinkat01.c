// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2006
 * Copyright (c) Cyril Hrubis 2014 <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2003-2023
 * Author: Yi Yang <yyangcdl@cn.ibm.com>
 */

/*\
 * [Description]
 *
 * Check the basic functionality of the readlinkat() system call.
 *
 * - readlinkat() passes if dirfd is directory file descriptor
 *   and the pathname is relative.
 * - readlinkat() passes if the pathname is abspath, then dirfd
 *   is ignored.
 * - readlinkat() passes if dirfd is the special value AT_FDCWD
 *   and the pathname is relative.
 * - readlinkat() passes if pathname is an empty string, in which
 *   case the call operates on the symbolic link referred to by dirfd.
 */

#include <stdlib.h>
#include <stdio.h>
#include "tst_test.h"
#include "lapi/fcntl.h"

#define TEST_FILE       "readlink_file"
#define TEST_SYMLINK    "readlink_symlink"

static int file_fd, dir_fd, dir_fd2;
static int fd_atcwd = AT_FDCWD;
static const char *abspath;
static const char *testsymlink;
static const char *emptypath;

static struct tcase {
	int *fd;
	const char **path;
} tcases[] = {
	{&dir_fd, &testsymlink},
	{&dir_fd, &abspath},
	{&file_fd, &abspath},
	{&fd_atcwd, &abspath},
	{&fd_atcwd, &testsymlink},
	{&dir_fd2, &emptypath},
};

static void verify_readlinkat(unsigned int i)
{
	char buf[1024];
	struct tcase *tc = &tcases[i];

	memset(buf, 0, sizeof(buf));

	TST_EXP_POSITIVE(readlinkat(*tc->fd, *tc->path, buf, sizeof(buf)),
		     "readlinkat(%d, %s, %s, %ld)",
		     *tc->fd, *tc->path, buf, sizeof(buf));

	if (strcmp(buf, TEST_FILE) == 0)
		tst_res(TPASS, "The filename in buffer is correct");
	else
		tst_res(TFAIL, "Wrong filename in buffer '%s'", buf);
}

static void setup(void)
{
	abspath = tst_tmpdir_mkpath(TEST_SYMLINK);

	file_fd = SAFE_OPEN(TEST_FILE, O_CREAT, 0600);
	SAFE_SYMLINK(TEST_FILE, TEST_SYMLINK);
	dir_fd = SAFE_OPEN(".", O_DIRECTORY);
	dir_fd2 = SAFE_OPEN(TEST_SYMLINK, O_PATH | O_NOFOLLOW);
}

static void cleanup(void)
{
	if (file_fd > -1)
		SAFE_CLOSE(file_fd);

	if (dir_fd > -1)
		SAFE_CLOSE(dir_fd);

	if (dir_fd2 > -1)
		SAFE_CLOSE(dir_fd2);
}

static struct tst_test test = {
	.test = verify_readlinkat,
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.bufs = (struct tst_buffers []) {
		{&testsymlink, .str = TEST_SYMLINK},
		{&emptypath, .str = ""},
		{},
	},
	.tcnt = ARRAY_SIZE(tcases),
};
