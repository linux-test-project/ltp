// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2012 Linux Test Project, Inc.
 * Copyright (C) 2023-2024 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * [Description]
 *
 * Verify that readahead() syscall fails with:
 *
 * - EBADF when fd is not a valid file descriptor or is not open for reading.
 * - EINVAL when fd does not refer to a file type to which readahead()
 *          can be applied.
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
	int fd[2];

	TST_EXP_FAIL(readahead(-1, 0, getpagesize()), EBADF,
	             "readahead() with fd = -1");

	SAFE_PIPE(fd);
	SAFE_CLOSE(fd[0]);
	SAFE_CLOSE(fd[1]);

	TST_EXP_FAIL(readahead(fd[0], 0, getpagesize()), EBADF,
	             "readahead() with invalid fd");
}

static void test_invalid_fd(struct tst_fd *fd)
{

	switch (fd->type) {
	/* These succeed */
	case TST_FD_FILE:
	case TST_FD_MEMFD:
	case TST_FD_MEMFD_SECRET:
	case TST_FD_PROC_MAPS:
		return;
	default:
		break;
	}

	int exp_errnos[] = {EBADF, EINVAL, ESPIPE};

	TST_EXP_FAIL_ARR(readahead(fd->fd, 0, getpagesize()), exp_errnos,
			"readahead() on %s", tst_fd_desc(fd));
}

static void test_readahead(void)
{
	test_bad_fd();

	TST_FD_FOREACH(fd)
		test_invalid_fd(&fd);
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
