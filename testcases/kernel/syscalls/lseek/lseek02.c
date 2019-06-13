// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * 06/2017 modified by Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

/*
 * DESCRIPTION
 * 1) lseek(2) fails and sets errno to EBADF when fd is invalid.
 * 2) lseek(2) fails ans sets errno to EINVAL when whence is invalid.
 * 3) lseek(2) fails and sets errno to ESPIPE when fd is associated
 *    with a pipe or FIFO.
 */

#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include "tst_test.h"

#define TFILE "tfile"
#define TFIFO1 "tfifo1"
#define TFIFO2 "tfifo2"

static int bad_fd = -1;
static int fd, pfd1, pfd2;
static int pfds[2];

static struct tcase {
	int *fd;
	int whence;
	int exp_err;
} tcases[] = {
	{&bad_fd, SEEK_SET, EBADF},
	{&bad_fd, SEEK_CUR, EBADF},
	{&bad_fd, SEEK_END, EBADF},
	{&fd, 5, EINVAL},
	{&fd, -1, EINVAL},
	{&fd, 7, EINVAL},
	{&pfd1, SEEK_SET, ESPIPE},
	{&pfd1, SEEK_CUR, ESPIPE},
	{&pfd1, SEEK_END, ESPIPE},
	{&pfds[0], SEEK_SET, ESPIPE},
	{&pfds[0], SEEK_CUR, ESPIPE},
	{&pfds[0], SEEK_END, ESPIPE},
	{&pfd2, SEEK_SET, ESPIPE},
	{&pfd2, SEEK_CUR, ESPIPE},
	{&pfd2, SEEK_END, ESPIPE},
};

static void verify_lseek(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(lseek(*tc->fd, (off_t) 1, tc->whence));
	if (TST_RET != (off_t) -1) {
		tst_res(TFAIL, "lseek(%d, 1, %d) succeeded unexpectedly",
			*tc->fd, tc->whence);
			return;
	}

	if (TST_ERR == tc->exp_err) {
		tst_res(TPASS | TTERRNO, "lseek(%d, 1, %d) failed as expected",
			*tc->fd, tc->whence);
	} else {
		tst_res(TFAIL | TTERRNO, "lseek(%d, 1, %d) failed "
			"unexpectedly, expected %s", *tc->fd, tc->whence,
			tst_strerrno(tc->exp_err));
	}
}

static void setup(void)
{
	fd = SAFE_OPEN(TFILE, O_RDWR | O_CREAT, 0777);
	SAFE_MKFIFO(TFIFO1, 0777);
	pfd1 = SAFE_OPEN(TFIFO1, O_RDWR, 0777);
	SAFE_PIPE(pfds);
	SAFE_MKNOD(TFIFO2, S_IFIFO | 0777, 0);
	pfd2 = SAFE_OPEN(TFIFO2, O_RDWR, 0777);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);

	if (pfd1 > 0)
		SAFE_CLOSE(pfd1);

	if (pfds[0] > 0)
		SAFE_CLOSE(pfds[0]);

	if (pfds[1] > 0)
		SAFE_CLOSE(pfds[1]);

	if (pfd2 > 0)
		SAFE_CLOSE(pfd2);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_lseek,
	.needs_tmpdir = 1,
};
