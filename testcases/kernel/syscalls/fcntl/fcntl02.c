// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */
 /*
  * Basic test for fcntl(2) using F_DUPFD argument.
  */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "tst_test.h"

static int fd;
static char fname[256];

static const int min_fds[] = {0, 1, 2, 3, 10, 100};

static void verify_fcntl(unsigned int n)
{
	int min_fd = min_fds[n];

	TEST(fcntl(fd, F_DUPFD, min_fd));

	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "fcntl(%s, F_DUPFD, %i) failed",
			fname, min_fd);
		return;
	}

	if (TST_RET < min_fd) {
		tst_res(TFAIL, "fcntl(%s, F_DUPFD, %i) returned %ld < %i",
			fname, min_fd, TST_RET, min_fd);
	}

	tst_res(TPASS, "fcntl(%s, F_DUPFD, %i) returned %ld",
		fname, min_fd, TST_RET);

	SAFE_CLOSE(TST_RET);
}

static void setup(void)
{
	sprintf(fname, "fcntl02_%d", getpid());
	fd = SAFE_OPEN(fname, O_RDWR | O_CREAT, 0700);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.test = verify_fcntl,
	.tcnt = ARRAY_SIZE(min_fds),
	.setup = setup,
	.cleanup = cleanup,
};
