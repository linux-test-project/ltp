// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *    AUTHOR		: William Roske
 *    CO-PILOT		: Dave Fenner
 */

#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "tst_test.h"

static char fname[255];
static int fd;
#define BUF "davef"

static void verify_fsync(void)
{
	unsigned int i;

	for (i = 0; i < 10; i++) {
		SAFE_WRITE(1, fd, BUF, sizeof(BUF));

		TEST(fsync(fd));

		if (TST_RET == -1)
			tst_res(TFAIL | TTERRNO, "fsync failed");
		else
			tst_res(TPASS, "fsync() returned %ld", TST_RET);
	}
}

static void setup(void)
{
	sprintf(fname, "tfile_%d", getpid());
	fd = SAFE_OPEN(fname, O_RDWR | O_CREAT, 0700);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.cleanup = cleanup,
	.setup = setup,
	.test_all = verify_fsync,
	.needs_tmpdir = 1,
	.needs_root = 1,
};
