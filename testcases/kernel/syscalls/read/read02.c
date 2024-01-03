// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Ported to LTP: Wayne Boyer
 * Copyright (c) 2017 Fujitsu Ltd.
 *	04/2017 Modified by Jinhui Huang
 */
/*
 * DESCRIPTION
 *	test 1:
 *	Read with an invalid file descriptor, and expect an EBADF.
 *
 *	test 2:
 *	The parameter passed to read is a directory, check if the errno is
 *	set to EISDIR.
 *
 *	test 3:
 *	Buf is outside the accessible address space, expect an EFAULT.
 *
 *	test 4:
 *	The file was opened with the O_DIRECT flag, and transfer sizes was not
 *	multiples of the logical block size of the file system, expect an
 *	EINVAL.
 *
 *	test 5:
 *	The file was opened with the O_DIRECT flag, and the alignment of the
 *	user buffer was not multiples of the logical block size of the file
 *	system, expect an EINVAL.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "tst_test.h"

static int badfd = -1;
static int fd2, fd3, fd4 = -1;
static char buf[BUFSIZ];
static void *bufaddr = buf;
static void *outside_buf = (void *)-1;
static void *addr4;
static void *addr5;
static long fs_type;

static struct tcase {
	int *fd;
	void **buf;
	size_t count;
	int exp_error;
} tcases[] = {
	{&badfd, &bufaddr, 1, EBADF},
	{&fd2, &bufaddr, 1, EISDIR},
	{&fd3, &outside_buf, 1, EFAULT},
	{&fd4, &addr4, 1, EINVAL},
	{&fd4, &addr5, 4096, EINVAL},
};

static void verify_read(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	if (tc->fd == &fd4 && *tc->fd == -1) {
		tst_res(TCONF, "O_DIRECT not supported on %s filesystem",
			tst_fs_type_name(fs_type));
		return;
	}

	TEST(read(*tc->fd, *tc->buf, tc->count));

	if (*tc->fd == fd4 && TST_RET >= 0) {
		tst_res(TPASS,
			"O_DIRECT unaligned reads fallbacks to buffered I/O");
		return;
	}

	if (TST_RET != -1) {
		tst_res(TFAIL, "read() succeeded unexpectedly");
		return;
	}

	if (TST_ERR == tc->exp_error) {
		tst_res(TPASS | TTERRNO, "read() failed as expected");
	} else {
		tst_res(TFAIL | TTERRNO, "read() failed unexpectedly, "
			"expected %s", tst_strerrno(tc->exp_error));
	}
}

static void setup(void)
{
	fd2 = SAFE_OPEN(".", O_DIRECTORY);

	SAFE_FILE_PRINTF("test_file", "A");

	fd3 = SAFE_OPEN("test_file", O_RDWR);

	outside_buf = SAFE_MMAP(0, 1, PROT_NONE,
				MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

	addr4 = SAFE_MEMALIGN(getpagesize(), (4096 * 10));
	addr5 = addr4 + 1;

	fs_type = tst_fs_type(".");
	if (fs_type != TST_TMPFS_MAGIC)
		fd4 = SAFE_OPEN("test_file", O_RDWR | O_DIRECT);
}

static void cleanup(void)
{
	free(addr4);

	if (fd4 > 0)
		SAFE_CLOSE(fd4);

	if (fd3 > 0)
		SAFE_CLOSE(fd3);

	if (fd2 > 0)
		SAFE_CLOSE(fd2);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_read,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
