// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
 * Author: Jinhui Huang <huangjh.jy@cn.fujitsu.com>
 */
/*
 * Description:
 * Check various errnos for pwritev2(2).
 * 1) pwritev2() fails and sets errno to EINVAL if iov_len is invalid.
 * 2) pwritev2() fails and sets errno to EINVAL if the vector count iovcnt is
 *    less than zero.
 * 3) pwritev2() fails and sets errno to EOPNOTSUPP if flag is invalid.
 * 4) pwritev2() fails and sets errno to EFAULT when writing data from invalid
 *    address.
 * 5) pwritev2() fails and sets errno to EBADF if file descriptor is invalid.
 * 6) pwritev2() fails and sets errno to EBADF if file descriptor is open for
 *    reading.
 * 7) pwritev2() fails and sets errno to ESPIPE if fd is associated with a pipe.
 */

#define _GNU_SOURCE
#include <sys/uio.h>
#include <unistd.h>

#include "tst_test.h"
#include "lapi/pwritev2.h"

#define CHUNK	64

static int fd1;
static int fd2;
static int fd3 = -1;
static int fd4[2];

static char buf[CHUNK];

static struct iovec wr_iovec1[] = {
	{buf, -1},
};

static struct iovec wr_iovec2[] = {
	{buf, CHUNK},
};

static struct iovec wr_iovec3[] = {
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
	{&fd1, wr_iovec1, 1, 0, 0, EINVAL},
	{&fd1, wr_iovec2, -1, 0, 0, EINVAL},
	{&fd1, wr_iovec2, 1, 1, -1, EOPNOTSUPP},
	{&fd1, wr_iovec3, 1, 0, 0, EFAULT},
	{&fd3, wr_iovec2, 1, 0, 0, EBADF},
	{&fd2, wr_iovec2, 1, 0, 0, EBADF},
	{&fd4[0], wr_iovec2, 1, 0, 0, ESPIPE},
};

static void verify_pwritev2(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(pwritev2(*tc->fd, tc->name, tc->count, tc->offset, tc->flag));

	if (TST_RET == 0) {
		tst_res(TFAIL, "pwritev2() succeeded unexpectedly");
		return;
	}

	if (TST_ERR == tc->exp_err) {
		tst_res(TPASS | TTERRNO, "pwritev2() failed as expected");
		return;
	}

	tst_res(TFAIL | TTERRNO, "pwritev2() failed unexpectedly, expected %s",
		tst_strerrno(tc->exp_err));
}

static void setup(void)
{
	fd1 = SAFE_OPEN("file1", O_RDWR | O_CREAT, 0644);
	SAFE_FTRUNCATE(fd1, getpagesize());
	fd2 = SAFE_OPEN("file2", O_RDONLY | O_CREAT, 0644);
	SAFE_PIPE(fd4);

	wr_iovec3[0].iov_base = tst_get_bad_addr(NULL);
}

static void cleanup(void)
{
	if (fd1 > 0)
		SAFE_CLOSE(fd1);

	if (fd2 > 0)
		SAFE_CLOSE(fd2);

	if (fd4[0] > 0)
		SAFE_CLOSE(fd4[0]);

	if (fd4[1] > 0)
		SAFE_CLOSE(fd4[1]);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_pwritev2,
	.needs_tmpdir = 1,
};
