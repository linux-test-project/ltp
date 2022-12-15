// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2001
 *   07/2001 Ported by Wayne Boyer
 */
/*
 * Description:
 * 1) lseek(2) fails and sets errno to EINVAL when whence is invalid.
 * 2) lseek(2) fails ans sets errno to EBADF when fd is not an open
 * file descriptor.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include "tst_test.h"

#define TEMP_FILE1 "tmp_file1"
#define TEMP_FILE2 "tmp_file2"

#define FILE_MODE S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define SEEK_TOP 10

static int fd1;
static int fd2;

static struct tcase {
	int *fd;
	int whence;
	int exp_err;
} tcases[] = {
	{&fd1, SEEK_TOP, EINVAL},
	{&fd2, SEEK_SET, EBADF},
};

static void verify_llseek(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(lseek(*tc->fd, (loff_t) 1, tc->whence));
	if (TST_RET != (off_t) -1) {
		tst_res(TFAIL, "lseek(%d, 1, %d) succeeded unexpectedly (%ld)",
			*tc->fd, tc->whence, TST_RET);
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
	fd1 = SAFE_OPEN(TEMP_FILE1, O_RDWR | O_CREAT, FILE_MODE);
	fd2 = SAFE_OPEN(TEMP_FILE2, O_RDWR | O_CREAT, FILE_MODE);
	close(fd2);
}

static struct tst_test test = {
	.setup = setup ,
	.needs_tmpdir = 1 ,
	.test = verify_llseek,
	.tcnt = ARRAY_SIZE(tcases),
};
