/*
 * Copyright (c) International Business Machines  Corp., 2002
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.
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
	if (TEST_RETURN == -1) {
		tst_res(TFAIL | TTERRNO, "pipe() failed unexpectedly");
		return;
	}

	TEST(write(fd[0], "A", 1));
	if (TEST_RETURN == -1 && errno == EBADF) {
		tst_res(TPASS | TTERRNO, "expected failure writing "
			"to read end of pipe");
	} else {
		tst_res(TFAIL | TTERRNO, "unexpected failure writing "
			"to read end of pipe");
	}

	TEST(read(fd[1], buf, 1));
	if (TEST_RETURN == -1 && errno == EBADF) {
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
