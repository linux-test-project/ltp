// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Author: Vatsal Avasthi
 *
 * Test Description:
 *  1) flock() returns -1 and sets error number to EBADF if the file descriptor
 *     is invalid.
 *  2) flock() returns -1 and sets error number to EINVAL if the argument
 *     operation does not include LOCK_SH,LOCK_EX,LOCK_UN.
 *  3) flock() returns -1 and sets error number to EINVAL if an invalid
 *     combination of locking modes is used i.e LOCK_SH with LOCK_EX
 */

#include <errno.h>
#include <sys/file.h>

#include "tst_test.h"

static int badfd = -1;
static int fd;

static struct tcase {
	int *fd;
	int operation;
	int exp_err;
} tcases[] = {
	{&badfd, LOCK_SH, EBADF},
	{&fd, LOCK_NB, EINVAL},
	{&fd, LOCK_SH | LOCK_EX, EINVAL},
};

static void verify_flock(unsigned n)
{
	struct tcase *tc = &tcases[n];

	fd = SAFE_OPEN("testfile", O_RDWR);
	TEST(flock(*tc->fd, tc->operation));
	if (TST_RET == 0) {
		tst_res(TFAIL | TTERRNO, "flock() succeeded unexpectedly");
		SAFE_CLOSE(fd);
		return;
	}

	if (tc->exp_err == TST_ERR) {
		tst_res(TPASS | TTERRNO, "flock() failed expectedly");
	} else {
		tst_res(TFAIL | TTERRNO, "flock() failed unexpectedly, "
			"expected %s", tst_strerrno(tc->exp_err));
	}

	SAFE_CLOSE(fd);
}

static void setup(void)
{
	int fd1;

	fd1 = SAFE_OPEN("testfile", O_CREAT | O_TRUNC | O_RDWR, 0666);
	SAFE_CLOSE(fd1);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_flock,
	.needs_tmpdir = 1,
	.setup = setup,
};
