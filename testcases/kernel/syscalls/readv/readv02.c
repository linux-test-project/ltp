// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Bull S.A. 2001
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 * 05/2002 Ported by Jacky Malcles
 */

/*\
 * [Description]
 *
 * Tests readv() failures:
 *
 * - EINVAL the sum of the iov_len values overflows an ssize_t value
 * - EFAULT buffer is outside the accessible address space
 * - EINVAL the vector count iovcnt is less than zero
 * - EISDIR the file descriptor is a directory
 * - EBADF  the file descriptor is not valid
 */

#include <sys/uio.h>
#include "tst_test.h"

#define K_1     1024
#define MODES   0700

#define CHUNK           64

static int badfd = -1;
static int fd_dir, fd_file;
static char buf1[K_1];
const char *TEST_DIR = "test_dir";
const char *TEST_FILE = "test_file";

static struct iovec invalid_iovec[] = {
	{buf1, -1},
	{buf1 + CHUNK, CHUNK},
	{buf1 + 2*CHUNK, CHUNK},
};

static struct iovec large_iovec[] = {
	{buf1, K_1},
	{buf1 + CHUNK, K_1},
	{buf1 + CHUNK*2, K_1},
};

static struct iovec efault_iovec[] = {
	{NULL, CHUNK},
	{buf1 + CHUNK, CHUNK},
	{buf1 + 2*CHUNK, CHUNK},
};

static struct iovec valid_iovec[] = {
	{buf1, CHUNK},
};

static struct tcase {
	int *fd;
	void *buf;
	int count;
	int exp_error;
} tcases[] = {
	{&fd_file, invalid_iovec, 1, EINVAL},
	{&fd_file, efault_iovec, 3, EFAULT},
	{&fd_file, large_iovec, -1, EINVAL},
	{&fd_dir, valid_iovec, 1, EISDIR},
	{&badfd, valid_iovec, 3, EBADF},
};

static void verify_readv(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TST_EXP_FAIL2(readv(*tc->fd, tc->buf, tc->count), tc->exp_error,
		"readv(%d, %p, %d)", *tc->fd, tc->buf, tc->count);
}

static void setup(void)
{
	SAFE_FILE_PRINTF(TEST_FILE, "test");

	fd_file = SAFE_OPEN(TEST_FILE, O_RDONLY);

	efault_iovec[0].iov_base = tst_get_bad_addr(NULL);

	SAFE_MKDIR(TEST_DIR, MODES);
	fd_dir = SAFE_OPEN(TEST_DIR, O_RDONLY);
}

static void cleanup(void)
{
	if (fd_file > 0)
		SAFE_CLOSE(fd_file);

	if (fd_dir > 0)
		SAFE_CLOSE(fd_dir);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_readv,
};
