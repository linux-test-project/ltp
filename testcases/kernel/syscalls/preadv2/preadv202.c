// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

/*
 * Description:
 * Check various errnos for preadv2(2).
 * 1) preadv2() fails and sets errno to EINVAL if iov_len is invalid.
 * 2) preadv2() fails and sets errno to EINVAL if the vector count iovcnt
 *    is less than zero.
 * 3) preadv2() fails and sets errno to EOPNOTSUPP if flag is invalid.
 * 4) preadv2() fails and sets errno to EFAULT when attempts to read into
 *    a invalid address.
 * 5) preadv2() fails and sets errno to EBADF if file descriptor is invalid.
 * 6) preadv2() fails and sets errno to EBADF if file descriptor is not
 *    open for reading.
 * 7) preadv2() fails and sets errno to EISDIR when fd refers to a directory.
 * 8) preadv2() fails and sets errno to ESPIPE if fd is associated with a pipe.
 */

#define _GNU_SOURCE
#include <sys/uio.h>
#include <unistd.h>

#include "tst_test.h"
#include "lapi/uio.h"

#define CHUNK           64

static int fd1;
static int fd2;
static int fd3 = -1;
static int fd4;
static int fd5[2];

static char buf[CHUNK];

static struct iovec rd_iovec1[] = {
	{buf, -1},
};

static struct iovec rd_iovec2[] = {
	{buf, CHUNK},
};

static struct iovec rd_iovec3[] = {
	{NULL, CHUNK},
};

static struct tcase {
	int *fd;
	struct iovec *name;
	int count;
	off_t offset;
	int flag;
	int exp_err;
} tcases[] = {
	{&fd1, rd_iovec1, 1, 0, 0, EINVAL},
	{&fd1, rd_iovec2, -1, 0, 0, EINVAL},
	{&fd1, rd_iovec2, 1, 1, -1, EOPNOTSUPP},
	{&fd1, rd_iovec3, 1, 0, 0, EFAULT},
	{&fd3, rd_iovec2, 1, 0, 0, EBADF},
	{&fd2, rd_iovec2, 1, 0, 0, EBADF},
	{&fd4, rd_iovec2, 1, 0, 0, EISDIR},
	{&fd5[0], rd_iovec2, 1, 0, 0, ESPIPE},
};

static void verify_preadv2(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(preadv2(*tc->fd, tc->name, tc->count, tc->offset, tc->flag));

	if (TST_RET == 0) {
		tst_res(TFAIL, "preadv2() succeeded unexpectedly");
		return;
	}

	if (TST_ERR == tc->exp_err) {
		tst_res(TPASS | TTERRNO, "preadv2() failed as expected");
		return;
	}

	tst_res(TFAIL | TTERRNO, "preadv2() failed unexpectedly, expected %s",
		tst_strerrno(tc->exp_err));
}

static void setup(void)
{
	fd1 = SAFE_OPEN("file1", O_RDWR | O_CREAT, 0644);
	SAFE_FTRUNCATE(fd1, getpagesize());
	fd2 = SAFE_OPEN("file2", O_WRONLY | O_CREAT, 0644);
	fd4 = SAFE_OPEN(".", O_RDONLY);
	SAFE_PIPE(fd5);

	rd_iovec3[0].iov_base = tst_get_bad_addr(NULL);
}

static void cleanup(void)
{
	if (fd1 > 0)
		SAFE_CLOSE(fd1);

	if (fd2 > 0)
		SAFE_CLOSE(fd2);

	if (fd4 > 0)
		SAFE_CLOSE(fd4);

	if (fd5[0] > 0)
		SAFE_CLOSE(fd5[0]);

	if (fd5[1] > 0)
		SAFE_CLOSE(fd5[1]);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_preadv2,
	.needs_tmpdir = 1,
};
