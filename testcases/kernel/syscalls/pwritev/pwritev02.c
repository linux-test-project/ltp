/*
* Copyright (c) 2015-2016 Fujitsu Ltd.
* Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of version 2 of the GNU General Public License as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it would be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* You should have received a copy of the GNU General Public License
* alone with this program.
*/

/*
* Test Name: pwritev02
*
* Description:
* 1) pwritev(2) fails if iov_len is invalid.
* 2) pwritev(2) fails if the vector count iovcnt is less than zero.
* 3) pwritev(2) fails if offset is negative.
* 4) pwritev(2) fails when attempts to write from a invalid address
* 5) pwritev(2) fails if file descriptor is invalid.
* 6) pwritev(2) fails if file descriptor is not open for writing.
* 7) pwritev(2) fails if fd is associated with a pipe.
*
* Expected Result:
* 1) pwritev(2) should return -1 and set errno to EINVAL.
* 2) pwritev(2) should return -1 and set errno to EINVAL.
* 3) pwritev(2) should return -1 and set errno to EINVAL.
* 4) pwritev(2) should return -1 and set errno to EFAULT.
* 5) pwritev(2) should return -1 and set errno to EBADF.
* 6) pwritev(2) should return -1 and set errno to EBADF.
* 7) pwritev(2) should return -1 and set errno to ESPIPE.
*/

#include <sys/uio.h>
#include <unistd.h>
#include "tst_test.h"
#include "pwritev.h"

#define CHUNK           64

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
	{(char *)-1, CHUNK},
};

static struct tcase {
	int *fd;
	struct iovec *name;
	int count;
	off_t offset;
	int exp_err;
} tcases[] = {
	{&fd1, wr_iovec1, 1, 0, EINVAL},
	{&fd1, wr_iovec2, -1, 0, EINVAL},
	{&fd1, wr_iovec2, 1, -1, EINVAL},
	{&fd1, wr_iovec3, 1, 0, EFAULT},
	{&fd3, wr_iovec2, 1, 0, EBADF},
	{&fd2, wr_iovec2, 1, 0, EBADF},
	{&fd4[1], wr_iovec2, 1, 0, ESPIPE}
};

static void verify_pwritev(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(pwritev(*tc->fd, tc->name, tc->count, tc->offset));
	if (TST_RET == 0) {
		tst_res(TFAIL, "pwritev() succeeded unexpectedly");
		return;
	}

	if (TST_ERR == tc->exp_err) {
		tst_res(TPASS | TTERRNO, "pwritev() failed as expected");
		return;
	}

	tst_res(TFAIL | TTERRNO, "pwritev() failed unexpectedly, expected %s",
		tst_strerrno(tc->exp_err));
}

static void setup(void)
{
	fd1 = SAFE_OPEN("file", O_RDWR | O_CREAT, 0644);
	SAFE_FTRUNCATE(fd1, getpagesize());
	fd2 = SAFE_OPEN("file", O_RDONLY | O_CREAT, 0644);
	SAFE_PIPE(fd4);
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
	.test = verify_pwritev,
	.min_kver = "2.6.30",
	.needs_tmpdir = 1,
};
