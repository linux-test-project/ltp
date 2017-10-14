/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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

#define TEMPFILE	"pwrite_file"
#define K1		1024

static char write_buf[K1];

static void setup(void);

static void test_espipe(void);
static void test_einval(void);
static void test_ebadf1(void);
static void test_ebadf2(void);

#if !defined(UCLINUX)
static void test_efault(void);
#endif

static void (*testfunc[])(void) = {
	test_espipe, test_einval, test_ebadf1, test_ebadf2,
#if !defined(UCLINUX)
	test_efault
#endif
};

static void verify_pwrite(unsigned int i)
{
	(*testfunc[i])();
}

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

static void setup(void)
{
	/* see the comment in the sighandler() function */
	/* call signal() to trap the signal generated */
	if (signal(SIGXFSZ, sighandler) == SIG_ERR)
		tst_brk(TBROK | TERRNO, "signal() failed");

	memset(write_buf, 'a', K1);
}

static void print_test_result(int err, int exp_errno)
{
	if (err == 0) {
		tst_res(TFAIL, "call succeeded unexpectedly");
		return;
	}

	if (err == exp_errno) {
		tst_res(TPASS, "pwrite failed as expected: %d - %s",
			err, tst_strerrno(err));
	} else {
		tst_res(TFAIL, "pwrite failed unexpectedly; expected: %d - %s"
			"return: %d - %s", exp_errno, tst_strerrno(exp_errno),
			err, tst_strerrno(err));
	}
}

static void test_espipe(void)
{
	int pipe_fds[2];

	SAFE_PIPE(pipe_fds);

	TEST(pwrite(pipe_fds[1], write_buf, K1, 0));

	print_test_result(TEST_ERRNO, ESPIPE);

	SAFE_CLOSE(pipe_fds[0]);
	SAFE_CLOSE(pipe_fds[1]);
}

static void test_einval(void)
{
	int fd;

	fd = SAFE_OPEN(TEMPFILE, O_RDWR | O_CREAT, 0666);

	/* the specified offset was invalid */
	TEST(pwrite(fd, write_buf, K1, -1));

	print_test_result(TEST_ERRNO, EINVAL);

	SAFE_CLOSE(fd);
}

static void test_ebadf1(void)
{
	int fd = -1;

	TEST(pwrite(fd, write_buf, K1, 0));

	print_test_result(TEST_ERRNO, EBADF);
}

static void test_ebadf2(void)
{
	int fd;

	fd = SAFE_OPEN(TEMPFILE, O_RDONLY | O_CREAT, 0666);

	TEST(pwrite(fd, write_buf, K1, 0));

	print_test_result(TEST_ERRNO, EBADF);

	SAFE_CLOSE(fd);
}

#if !defined(UCLINUX)
static void test_efault(void)
{
	int fd;
	char *buf = sbrk(0);

	fd = SAFE_OPEN(TEMPFILE, O_RDWR | O_CREAT, 0666);

	TEST(pwrite(fd, buf, K1, 0));

	print_test_result(TEST_ERRNO, EFAULT);

	SAFE_CLOSE(fd);
}
#endif

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.test = verify_pwrite,
	.tcnt = ARRAY_SIZE(testfunc),
};
