// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Copyright (c) Linux Test Project, 2003-2023
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 */

/*\
 * [Description]
 *
 * - readlinkat() fails with EINVAL if the bufsiz is 0.
 *
 * - readlinkat() fails with EINVAL if the named file is not a symbolic link.
 *
 * - readlinkat() fails with ENOTDIR if the component of the path prefix is
 *   not a directory.
 *
 * - readlinkat() fails with ENOTDIR if the pathname is relative and
 *   dirfd is a file descriptor referring to a file other than a directory.
 *
 * - readlinkat() fails with EBADF if the file descriptor is invalid.
 *
 * - readlinkat() fails with ENOENT when the pathname does not exists.
 */

#include "tst_test.h"

#define TEST_FILE	"test_file"
#define SYMLINK_FILE	"symlink_file"
#define BUFF_SIZE	256

static int file_fd, dir_fd;
static int fd_invalid = -1;

static struct tcase {
	int *fd;
	const char *pathname;
	size_t bufsiz;
	int exp_errno;
} tcases[] = {
	{&dir_fd, SYMLINK_FILE, 0, EINVAL},
	{&dir_fd, TEST_FILE, BUFF_SIZE, EINVAL},
	{&file_fd, SYMLINK_FILE, BUFF_SIZE, ENOTDIR},
	{&dir_fd, "test_file/test_file", BUFF_SIZE, ENOTDIR},
	{&fd_invalid, SYMLINK_FILE, BUFF_SIZE, EBADF},
	{&dir_fd, "does_not_exists", BUFF_SIZE, ENOENT},
};

static void verify_readlinkat(unsigned int i)
{
	char buf[BUFF_SIZE];
	struct tcase *tc = &tcases[i];

	memset(buf, 0, sizeof(buf));

	TST_EXP_FAIL(readlinkat(*tc->fd, tc->pathname, buf, tc->bufsiz),
		     tc->exp_errno, "readlinkat(%d, %s, NULL, %ld)",
		     *tc->fd, tc->pathname, tc->bufsiz);
}

static void setup(void)
{
	dir_fd = SAFE_OPEN(".", O_RDONLY);

	file_fd = SAFE_OPEN(TEST_FILE, O_RDWR | O_CREAT, 0644);

	SAFE_SYMLINK(TEST_FILE, SYMLINK_FILE);
}

static void cleanup(void)
{
	if (file_fd > -1)
		SAFE_CLOSE(file_fd);

	if (dir_fd > -1)
		SAFE_CLOSE(dir_fd);
}

static struct tst_test test = {
	.test = verify_readlinkat,
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.tcnt = ARRAY_SIZE(tcases),
};
