// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines Corp., 2001
 *                 Linux Test Project, 2016
 */

/*
 * DESCRIPTION
 *	Testcase to check the basic functionality of writev(2) system call.
 *	Create IO vectors and attempt to writev various components of it.
 */

#include <errno.h>
#include <signal.h>
#include <sys/uio.h>
#include "tst_test.h"

#define	CHUNK		64
#define	TESTFILE	"writev_data_file"

static int valid_fd;
static int invalid_fd = -1;
static int pipe_fd[2];

static char buf[CHUNK * 4];

struct iovec iovec_badlen[] = {
	{ buf, -1 },
	{ buf + CHUNK, CHUNK },
	{ buf + CHUNK * 2, CHUNK },
};

struct iovec iovec_simple[] = {
	{ buf, CHUNK },
};

struct iovec iovec_zero_null[] = {
	{ buf, CHUNK },
	{ buf + CHUNK, 0 },
	{ NULL, 0 },
	{ NULL, 0 }
};

struct testcase_t {
	const char *desc;
	int *pfd;
	struct iovec (*piovec)[];
	int iovcnt;
	int exp_ret;
	int exp_errno;
} testcases[] = {
	{
		.desc = "invalid iov_len",
		.pfd = &valid_fd,
		.piovec = &iovec_badlen,
		.iovcnt = ARRAY_SIZE(iovec_badlen),
		.exp_ret = -1,
		.exp_errno = EINVAL
	},
	{
		.desc = "invalid fd",
		.pfd = &invalid_fd,
		.piovec = &iovec_simple,
		.iovcnt = ARRAY_SIZE(iovec_simple),
		.exp_ret = -1,
		.exp_errno = EBADF
	},
	{
		.desc = "invalid iovcnt",
		.pfd = &valid_fd,
		.piovec = &iovec_simple,
		.iovcnt = -1,
		.exp_ret = -1,
		.exp_errno = EINVAL
	},
	{
		.desc = "zero iovcnt",
		.pfd = &valid_fd,
		.piovec = &iovec_simple,
		.iovcnt = 0,
		.exp_ret = 0,
	},
	{
		.desc = "NULL and zero length iovec",
		.pfd = &valid_fd,
		.piovec = &iovec_zero_null,
		.iovcnt = ARRAY_SIZE(iovec_zero_null),
		.exp_ret = CHUNK,
	},
	{
		.desc = "write to closed pipe",
		.pfd = &(pipe_fd[1]),
		.piovec = &iovec_simple,
		.iovcnt = ARRAY_SIZE(iovec_simple),
		.exp_ret = -1,
		.exp_errno = EPIPE,
	},
};

void setup(void)
{
	sigset_t block_mask;

	sigemptyset(&block_mask);
	sigaddset(&block_mask, SIGPIPE);
	sigprocmask(SIG_BLOCK, &block_mask, NULL);

	valid_fd = SAFE_OPEN(TESTFILE, O_RDWR | O_CREAT, 0644);

	SAFE_PIPE(pipe_fd);
	SAFE_CLOSE(pipe_fd[0]);
}

static void test_writev(unsigned int i)
{
	struct testcase_t *tcase = &testcases[i];
	int ret;

	TEST(writev(*(tcase->pfd), *(tcase->piovec), tcase->iovcnt));

	ret = (TST_RET == tcase->exp_ret);
	if (TST_RET < 0 || tcase->exp_ret < 0) {
		ret &= (TST_ERR == tcase->exp_errno);
		tst_res((ret ? TPASS : TFAIL),
			"%s, expected: %d (%s), got: %ld (%s)", tcase->desc,
			tcase->exp_ret, tst_strerrno(tcase->exp_errno),
			TST_RET, tst_strerrno(TST_ERR));
	} else {
		tst_res((ret ? TPASS : TFAIL),
			"%s, expected: %d, got: %ld", tcase->desc,
			tcase->exp_ret, TST_RET);
	}
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.test = test_writev,
	.tcnt = ARRAY_SIZE(testcases),
};
