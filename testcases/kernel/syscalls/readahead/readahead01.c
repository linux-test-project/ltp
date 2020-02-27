// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2012 Linux Test Project, Inc.
 */

/*
 * errno tests for readahead() syscall
 */
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include "config.h"
#include "tst_test.h"
#include "lapi/syscalls.h"

#if defined(__NR_readahead)
static void check_ret(long expected_ret)
{
	if (expected_ret == TST_RET) {
		tst_res(TPASS, "expected ret success - "
			"returned value = %ld", TST_RET);
		return;
	}
	tst_res(TFAIL, "unexpected failure - "
		"returned value = %ld, expected: %ld",
		TST_RET, expected_ret);
}

static void check_errno(long expected_errno)
{
	if (TST_ERR == expected_errno) {
		tst_res(TPASS | TTERRNO, "expected failure");
		return;
	}

	if (TST_ERR == 0)
		tst_res(TFAIL, "call succeeded unexpectedly");
	else
		tst_res(TFAIL | TTERRNO, "unexpected failure - "
			"expected = %ld : %s, actual",
			expected_errno, strerror(expected_errno));
}

static void test_bad_fd(void)
{
	char tempname[PATH_MAX] = "readahead01_XXXXXX";
	int fd;

	tst_res(TINFO, "test_bad_fd -1");
	TEST(readahead(-1, 0, getpagesize()));
	check_ret(-1);
	check_errno(EBADF);

	tst_res(TINFO, "test_bad_fd O_WRONLY");
	fd = mkstemp(tempname);
	if (fd == -1)
		tst_res(TFAIL | TERRNO, "mkstemp failed");
	SAFE_CLOSE(fd);
	fd = SAFE_OPEN(tempname, O_WRONLY);
	TEST(readahead(fd, 0, getpagesize()));
	check_ret(-1);
	check_errno(EBADF);
	SAFE_CLOSE(fd);
	unlink(tempname);
}

static void test_invalid_fd(void)
{
	int fd[2];

	tst_res(TINFO, "test_invalid_fd pipe");
	SAFE_PIPE(fd);
	TEST(readahead(fd[0], 0, getpagesize()));
	check_ret(-1);
	check_errno(EINVAL);
	SAFE_CLOSE(fd[0]);
	SAFE_CLOSE(fd[1]);

	tst_res(TINFO, "test_invalid_fd socket");
	fd[0] = SAFE_SOCKET(AF_INET, SOCK_STREAM, 0);
	TEST(readahead(fd[0], 0, getpagesize()));
	check_ret(-1);
	check_errno(EINVAL);
	SAFE_CLOSE(fd[0]);
}

void test_readahead(void)
{
	test_bad_fd();
	test_invalid_fd();
}

static void setup(void)
{
	/* check if readahead syscall is supported */
	tst_syscall(__NR_readahead, 0, 0, 0);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.test_all = test_readahead,
};

#else /* __NR_readahead */
	TST_TEST_TCONF("System doesn't support __NR_readahead");
#endif
