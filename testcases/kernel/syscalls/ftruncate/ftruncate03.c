// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 * Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
 * Author: Jay Huie, Robbie Williamson
 */

/*\
 * Verify that ftruncate(2) system call returns appropriate error number:
 *
 * 1. EINVAL -- the file is a socket
 * 2. EINVAL -- the file descriptor was opened with O_RDONLY
 * 3. EINVAL -- the length is negative
 * 4. EBADF -- the file descriptor is invalid
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>

#include "tst_test.h"

#define TESTFILE1	"testfile1"
#define TESTFILE2	"testfile2"

static int sock_fd, read_fd, fd;
static int bad_fd = -1;

static struct tcase {
	int *fd;
	off_t length;
	int exp_errno;
} tcases[] = {
	{&sock_fd, 4, EINVAL},
	{&read_fd, 4, EINVAL},
	{&fd, -1, EINVAL},
	{&bad_fd, 4, EBADF},
};

static void verify_ftruncate(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(ftruncate(*tc->fd, tc->length));
	if (TST_RET != -1) {
		tst_res(TFAIL, "ftruncate() succeeded unexpectedly and got %ld",
			TST_RET);
		return;
	}

	if (TST_ERR == tc->exp_errno) {
		tst_res(TPASS | TTERRNO, "ftruncate() failed expectedly");
	} else {
		tst_res(TFAIL | TTERRNO,
			"ftruncate() failed unexpectedly, got %s, expected",
			tst_strerrno(tc->exp_errno));
	}
}

static void setup(void)
{
	sock_fd = SAFE_SOCKET(PF_INET, SOCK_STREAM, 0);

	read_fd = SAFE_OPEN(TESTFILE1, O_RDONLY | O_CREAT, 0644);

	fd = SAFE_OPEN(TESTFILE2, O_RDWR | O_CREAT, 0644);
}

static void cleanup(void)
{
	if (sock_fd > 0)
		SAFE_CLOSE(sock_fd);

	if (read_fd > 0)
		SAFE_CLOSE(read_fd);

	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_ftruncate,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
