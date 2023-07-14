// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2002
 * Copyright (c) 2003-2023 Linux Test Project
 */

/*\
 * [Description]
 *
 * Verify that, an attempt to write to the read end of a pipe fails with EBADF
 * and an attempt to read from the write end of a pipe also fails with EBADF.
 */

#include "tst_test.h"

static int fd[2];

static void verify_pipe(void)
{
	char buf[] = "abcdef";

	SAFE_PIPE(fd);

	TST_EXP_FAIL2(write(fd[0], "A", 1), EBADF);
	TST_EXP_FAIL2(read(fd[1], buf, 1), EBADF);

	SAFE_CLOSE(fd[0]);
	SAFE_CLOSE(fd[1]);
}

static void cleanup(void)
{
	if (fd[0] > 0)
		SAFE_CLOSE(fd[0]);
	if (fd[1] > 0)
		SAFE_CLOSE(fd[1]);
}

static struct tst_test test = {
	.test_all = verify_pipe,
	.cleanup = cleanup
};
