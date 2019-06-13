// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */
 /*
  * Basic test for fcntl(2) using F_GETFL argument.
  */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "tst_test.h"

static int fd;
static char fname[255];

static void verify_fcntl(void)
{
	TEST(fcntl(fd, F_GETFL, 0));

	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "fcntl(%s, F_GETFL, 0) failed",
			fname);
		return;
	}

	if ((TST_RET & O_ACCMODE) != O_RDWR) {
		tst_res(TFAIL, "fcntl(%s, F_GETFL, 0) returned wrong "
			"access mode %li, expected %i", fname,
			TST_RET & O_ACCMODE, O_RDWR);
		return;
	}

	tst_res(TPASS, "fcntl(%s, F_GETFL, 0) returned %lx",
		fname, TST_RET);
}

static void setup(void)
{
	sprintf(fname, "fcntl04_%d", getpid());
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
