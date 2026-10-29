// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 */

/*\
 * Verify :manpage:`close(2)` failure cases:
 *
 * 1) close(-1) returns EBADF.
 * 2) closing the same fd twice returns EBADF on the second call.
 */

#include <errno.h>
#include <fcntl.h>

#include "tst_test.h"

static int fd_invalid = -1;
static int fd_closed = -1;

static struct tcase {
	const char *desc;
	int *fd;
	int exp_errno;
} tcases[] = {
	{ "close(-1)", &fd_invalid, EBADF },
	{ "close same fd twice", &fd_closed, EBADF },
};

static void verify_close(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	TST_EXP_FAIL(close(*tc->fd), tc->exp_errno, "%s", tc->desc);
}

static void setup(void)
{
	fd_closed = SAFE_OPEN("close02", O_CREAT | O_RDWR, 0600);
	if (close(fd_closed) == -1)
		tst_brk(TBROK | TERRNO, "close(%d) failed", fd_closed);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_close,
};
