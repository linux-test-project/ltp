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

static void test_bad_fd(void)
{
	char tempname[PATH_MAX] = "readahead01_XXXXXX";
	int fd;

	tst_res(TINFO, "%s -1", __func__);
	TST_EXP_FAIL(readahead(-1, 0, getpagesize()), EBADF);

	tst_res(TINFO, "%s O_WRONLY", __func__);
	fd = mkstemp(tempname);
	if (fd == -1)
		tst_res(TFAIL | TERRNO, "mkstemp failed");
	SAFE_CLOSE(fd);
	fd = SAFE_OPEN(tempname, O_WRONLY);
	TST_EXP_FAIL(readahead(fd, 0, getpagesize()), EBADF);
	SAFE_CLOSE(fd);
	unlink(tempname);
}

static void test_invalid_fd(void)
{
	int fd[2];

	tst_res(TINFO, "%s pipe", __func__);
	SAFE_PIPE(fd);
	TST_EXP_FAIL(readahead(fd[0], 0, getpagesize()), EINVAL);
	SAFE_CLOSE(fd[0]);
	SAFE_CLOSE(fd[1]);

	tst_res(TINFO, "%s socket", __func__);
	fd[0] = SAFE_SOCKET(AF_INET, SOCK_STREAM, 0);
	TST_EXP_FAIL(readahead(fd[0], 0, getpagesize()), EINVAL);
	SAFE_CLOSE(fd[0]);
}

static void test_readahead(void)
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
