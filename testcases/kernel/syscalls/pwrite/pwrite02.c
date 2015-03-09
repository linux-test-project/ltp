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
#include <fcntl.h>

#include "test.h"
#include "safe_macros.h"

#define TEMPFILE	"pwrite_file"
#define K1		1024

TCID_DEFINE(pwrite02);

static char write_buf[K1];

static void setup(void);
static void cleanup(void);

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

int TST_TOTAL = ARRAY_SIZE(testfunc);

int main(int ac, char **av)
{
	int i, lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			(*testfunc[i])();
	}

	cleanup();
	tst_exit();
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
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* see the comment in the sighandler() function */
	/* call signal() to trap the signal generated */
	if (signal(SIGXFSZ, sighandler) == SIG_ERR)
		tst_brkm(TBROK, cleanup, "signal() failed");

	TEST_PAUSE;

	tst_tmpdir();

	memset(write_buf, 'a', K1);
}

static void print_test_result(int err, int exp_errno)
{
	if (err == 0) {
		tst_resm(TFAIL, "call succeeded unexpectedly");
		return;
	}

	if (err == exp_errno) {
		tst_resm(TPASS, "pwrite failed as expected: %d - %s",
			 err, strerror(err));
	} else {
		tst_resm(TFAIL, "pwrite failed unexpectedly; expected: %d - %s"
			 "return: %d - %s", exp_errno, strerror(exp_errno),
			 err, strerror(err));
	}
}

static void test_espipe(void)
{
	int pipe_fds[2];

	SAFE_PIPE(cleanup, pipe_fds);

	TEST(pwrite(pipe_fds[1], write_buf, K1, 0));

	print_test_result(errno, ESPIPE);

	SAFE_CLOSE(cleanup, pipe_fds[0]);
	SAFE_CLOSE(cleanup, pipe_fds[1]);
}

static void test_einval(void)
{
	int fd;

	fd = SAFE_OPEN(cleanup, TEMPFILE, O_RDWR | O_CREAT, 0666);

	/* the specified offset was invalid */
	TEST(pwrite(fd, write_buf, K1, -1));

	print_test_result(errno, EINVAL);

	SAFE_CLOSE(cleanup, fd);
}

static void test_ebadf1(void)
{
	int fd = -1;

	TEST(pwrite(fd, write_buf, K1, 0));

	print_test_result(errno, EBADF);
}

static void test_ebadf2(void)
{
	int fd;

	fd = SAFE_OPEN(cleanup, TEMPFILE, O_RDONLY | O_CREAT, 0666);

	TEST(pwrite(fd, write_buf, K1, 0));

	print_test_result(errno, EBADF);

	SAFE_CLOSE(cleanup, fd);
}

#if !defined(UCLINUX)
static void test_efault(void)
{
	int fd;
	char *buf = sbrk(0);

	fd = SAFE_OPEN(cleanup, TEMPFILE, O_RDWR | O_CREAT, 0666);

	TEST(pwrite(fd, buf, K1, 0));

	print_test_result(errno, EFAULT);

	SAFE_CLOSE(cleanup, fd);
}
#endif

static void cleanup(void)
{
	tst_rmdir();
}
