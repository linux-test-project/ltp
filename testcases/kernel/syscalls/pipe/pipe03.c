// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2002
 */

/*
 * Make sure that writing to the read end of a pipe and reading from
 * the write end of a pipe both fail.
 */

#include <unistd.h>
#include <errno.h>
#include "tst_test.h"

static int fd[2];

static void verify_pipe(void)
{
	char buf[2];

	TEST(pipe(fd));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "pipe() failed unexpectedly");
		return;
	}

	TEST(write(fd[0], "A", 1));
	if (TST_RET == -1 && errno == EBADF) {
		tst_res(TPASS | TTERRNO, "expected failure writing "
			"to read end of pipe");
	} else {
		tst_res(TFAIL | TTERRNO, "unexpected failure writing "
			"to read end of pipe");
	}

	TEST(read(fd[1], buf, 1));
	if (TST_RET == -1 && errno == EBADF) {
		tst_res(TPASS | TTERRNO, "expected failure reading "
			"from write end of pipe");
	} else {
		tst_res(TFAIL | TTERRNO, "unexpected failure reading "
			"from write end of pipe");
	}

	SAFE_CLOSE(fd[0]);
	SAFE_CLOSE(fd[1]);
}

static struct tst_test test = {
	.test_all = verify_pipe,
};
