// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) 2020 SUSE LLC
 *
 * 03/30/1992 AUTHOR: William Roske CO-PILOT: Dave Fenner
 *
 */

/*\
 * [Description]
 *
 * Verify that dup(2) syscall executes successfully and allocates
 * a new file descriptor which refers to the same open file as oldfd.
 */

#include "tst_test.h"

static int fd;
static struct stat buf1, buf2;

static void verify_dup(void)
{
	TST_EXP_FD(dup(fd));

	SAFE_FSTAT(TST_RET, &buf2);
	TST_EXP_EQ_LU(buf1.st_ino, buf2.st_ino);

	SAFE_CLOSE(TST_RET);
}

static void setup(void)
{
	fd = SAFE_OPEN("dupfile", O_RDWR | O_CREAT, 0700);
	SAFE_FSTAT(fd, &buf1);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = verify_dup,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
