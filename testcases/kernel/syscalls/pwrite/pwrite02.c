// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *   Copyright (c) International Business Machines Corp., 2001
 */

/*
 * Test Description:
 *  Verify that,
 *   1) pwrite() fails when attempted to write to an unnamed pipe,
 *      returns ESPIPE.
 *   2) pwrite() fails if the specified offset position was invalid,
 *	returns EINVAL.
 *   3) pwrite() fails if fd is not a valid file descriptor,
 *	returns EBADF.
 *   4) pwrite() fails if fd is not open for writing, return EBADF.
 *   5) pwrite() fails when attempted to write with buf outside
 *      accessible address space, returns EFAULT.
 */

#define _XOPEN_SOURCE 500

#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "tst_test.h"

#define TEMPFILE "pwrite_file"
#define BS 1024

static int fd;
static int fd_ro;
static int inv_fd = -1;
static int pipe_fds[2];
static char buf[BS];

static struct tcase {
	void *buf;
	size_t size;
	int *fd;
	off_t off;
	int exp_errno;
} tcases[] = {
	{buf, sizeof(buf), &pipe_fds[1], 0, ESPIPE},
	{buf, sizeof(buf), &fd, -1, EINVAL},
	{buf, sizeof(buf), &inv_fd, 0, EBADF},
	{buf, sizeof(buf), &fd_ro, 0, EBADF},
	{NULL, sizeof(buf), &fd, 0, EFAULT},
};

/*
 * sighandler - handle SIGXFSZ
 *
 * This is here to start looking at a failure in test case #2.  This
 * test case passes on a machine running RedHat 6.2 but it will fail
 * on a machine running RedHat 7.1.
 */
static void sighandler(int sig)
{
	int ret;

	if (sig != SIGXFSZ) {
		ret = write(STDOUT_FILENO, "get wrong signal\n",
		            sizeof("get wrong signal\n"));
	} else {
		ret = write(STDOUT_FILENO, "caught SIGXFSZ\n",
		            sizeof("caught SIGXFSZ\n"));
	}

	(void)ret;
}

static void verify_pwrite(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	TEST(pwrite(*tc->fd, tc->buf, BS, tc->off));

	if (TST_RET >= 0) {
		tst_res(TFAIL, "call succeeded unexpectedly");
		return;
	}

	if (TST_ERR != tc->exp_errno) {
		tst_res(TFAIL | TTERRNO,
			"pwrite failed unexpectedly, expected %s",
			tst_strerrno(tc->exp_errno));
	}

	tst_res(TPASS | TTERRNO, "pwrite failed as expected");
}

static void setup(void)
{
	SAFE_SIGNAL(SIGXFSZ, sighandler);

	SAFE_PIPE(pipe_fds);

	fd = SAFE_OPEN(TEMPFILE, O_RDWR | O_CREAT, 0666);
	fd_ro = SAFE_OPEN(TEMPFILE, O_RDONLY | O_CREAT, 0666);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);

	if (fd_ro > 0)
		SAFE_CLOSE(fd_ro);

	if (pipe_fds[0] > 0)
		SAFE_CLOSE(pipe_fds[0]);

	if (pipe_fds[1] > 0)
		SAFE_CLOSE(pipe_fds[1]);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_pwrite,
	.tcnt = ARRAY_SIZE(tcases),
};
