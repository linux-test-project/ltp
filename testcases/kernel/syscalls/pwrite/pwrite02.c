// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 */

/*\
 * Test basic error handling of the pwrite syscall.
 *
 * - ESPIPE when attempted to write to an unnamed pipe
 * - EINVAL the specified offset position was invalid
 * - EBADF fd is not a valid file descriptor
 * - EBADF fd is not open for writing
 * - EFAULT when attempted to write with buf outside accessible address space
 */

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

	TST_EXP_FAIL2(pwrite(*tc->fd, tc->buf, BS, tc->off), tc->exp_errno,
		"pwrite(%d, %d, %ld)", *tc->fd, BS, tc->off);
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
