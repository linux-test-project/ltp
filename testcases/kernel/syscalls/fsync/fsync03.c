// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) Wayne Boyer, International Business Machines  Corp., 2001
 *   Copyright (c) 2019 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * Verify that, fsync(2) returns -1 and sets errno to
 *
 * - EINVAL if calling fsync() on a pipe(fd).
 * - EINVAL if calling fsync() on a socket(fd).
 * - EBADF if calling fsync() on a closed fd.
 * - EBADF if calling fsync() on an invalid fd.
 * - EINVAL if calling fsync() on a fifo(fd).
 */

#include <unistd.h>
#include <errno.h>
#include "tst_test.h"

#define FIFO_PATH "fifo"

static int fifo_rfd, fifo_wfd;
static int pipe_fd[2];
static int sock_fd, bad_fd = -1;

static const struct test_case {
	int *fd;
	int error;
} testcase_list[] = {
	/* EINVAL - fsync() on pipe should not succeed. */
	{&pipe_fd[1], EINVAL},
	/* EINVAL - fsync() on socket should not succeed. */
	{&sock_fd, EINVAL},
	/* EBADF - fd is closed */
	{&pipe_fd[0], EBADF},
	/* EBADF - fd is invalid (-1) */
	{&bad_fd, EBADF},
	/* EINVAL - fsync() on fifo should not succeed. */
	{&fifo_wfd, EINVAL},
};

static void setup(void)
{
	SAFE_MKFIFO(FIFO_PATH, 0644);
	SAFE_PIPE(pipe_fd);

	// FIFO must be opened for reading first, otherwise
	// open(fifo, O_WRONLY) will block.
	fifo_rfd = SAFE_OPEN(FIFO_PATH, O_RDONLY | O_NONBLOCK);
	fifo_wfd = SAFE_OPEN(FIFO_PATH, O_WRONLY);
	sock_fd = SAFE_SOCKET(AF_UNIX, SOCK_STREAM, 0);

	// Do not open any file descriptors after this line unless you close
	// them before the next test run.
	SAFE_CLOSE(pipe_fd[0]);
}

static void test_fsync(unsigned int n)
{
	const struct test_case *tc = testcase_list + n;

	TEST(fsync(*tc->fd));

	if (TST_RET != -1) {
		tst_res(TFAIL, "fsync() returned unexpected value %ld",
			TST_RET);
	} else if (TST_ERR != tc->error) {
		tst_res(TFAIL | TTERRNO, "fsync(): unexpected error");
	} else {
		tst_res(TPASS | TTERRNO, "fsync() failed as expected");
	}
}

static void cleanup(void)
{
	SAFE_CLOSE(fifo_wfd);
	SAFE_CLOSE(fifo_rfd);
	SAFE_CLOSE(pipe_fd[1]);
	SAFE_CLOSE(sock_fd);
}

static struct tst_test test = {
	.test = test_fsync,
	.tcnt = ARRAY_SIZE(testcase_list),
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup
};
