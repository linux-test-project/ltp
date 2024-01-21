// SPDX-License-Identifier: GPL-2.0-or-later
/*
* Copyright (c) 2015-2016 Fujitsu Ltd.
* Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
*/

/*\
 * [Description]
 *
 * - EINVAL when iov_len is invalid.
 * - EINVAL when the vector count iovcnt is less than zero.
 * - EINVAL when offset is negative.
 * - EFAULT when attempts to read into a invalid address.
 * - EBADF when file descriptor is invalid.
 * - EBADF when file descriptor is not open for reading.
 * - EISDIR when fd refers to a directory.
 * - ESPIPE when fd is associated with a pipe.
 */

#define _GNU_SOURCE

#include <sys/uio.h>
#include <unistd.h>
#include "tst_test.h"
#include "preadv.h"

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
	{(char *)-1, CHUNK},
};

static struct tcase {
	int *fd;
	struct iovec *name;
	int count;
	off_t offset;
	int exp_err;
} tcases[] = {
	{&fd1, rd_iovec1, 1, 0, EINVAL},
	{&fd1, rd_iovec2, -1, 0, EINVAL},
	{&fd1, rd_iovec2, 1, -1, EINVAL},
	{&fd1, rd_iovec3, 1, 0, EFAULT},
	{&fd3, rd_iovec2, 1, 0, EBADF},
	{&fd2, rd_iovec2, 1, 0, EBADF},
	{&fd4, rd_iovec2, 1, 0, EISDIR},
	{&fd5[0], rd_iovec2, 1, 0, ESPIPE}
};

static void verify_preadv(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(preadv(*tc->fd, tc->name, tc->count, tc->offset));

	if (TST_RET == 0) {
		tst_res(TFAIL, "preadv() succeeded unexpectedly");
		return;
	}

	if (TST_ERR == tc->exp_err) {
		tst_res(TPASS | TTERRNO, "preadv() failed as expected");
		return;
	}

	tst_res(TFAIL | TTERRNO, "preadv() failed unexpectedly, expected %s",
		tst_strerrno(tc->exp_err));
}

static void setup(void)
{
	fd1 = SAFE_OPEN("file1", O_RDWR | O_CREAT, 0644);
	SAFE_FTRUNCATE(fd1, getpagesize());
	fd2 = SAFE_OPEN("file2", O_WRONLY | O_CREAT, 0644);
	fd4 = SAFE_OPEN(".", O_RDONLY);
	SAFE_PIPE(fd5);
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
	.test = verify_preadv,
	.needs_tmpdir = 1,
};
