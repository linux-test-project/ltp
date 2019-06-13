// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */
 /*
  * Basic test for fcntl(2) using F_GETFD argument.
  */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "tst_test.h"

static char fname[255];
static int fd;

static void verify_fcntl(void)
{
	TEST(fcntl(fd, F_GETFD, 0));

	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "fcntl(%s, F_GETFD, 0) failed",
			fname);
		return;
	}

	tst_res(TPASS, "fcntl(%s, F_GETFD, 0) returned %ld",
		fname, TST_RET);
}

static void setup(void)
{
	sprintf(fname, "fcntl03_%d", getpid());
	fd = SAFE_OPEN(fname, O_RDWR | O_CREAT, 0700);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.test_all = verify_fcntl,
	.setup = setup,
	.cleanup = cleanup,
};
