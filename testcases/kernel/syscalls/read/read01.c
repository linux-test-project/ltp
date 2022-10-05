// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) 2017 Fujitsu Ltd.
 */

#include <errno.h>
#include "tst_test.h"

#define SIZE 512

static int fd;
static char buf[SIZE];

static void verify_read(void)
{
	SAFE_LSEEK(fd, 0, SEEK_SET);

	TEST(read(fd, buf, SIZE));

	if (TST_RET == -1)
		tst_res(TFAIL | TTERRNO, "read(2) failed");
	else
		tst_res(TPASS, "read(2) returned %ld", TST_RET);
}

static void setup(void)
{
	memset(buf, '*', SIZE);
	fd = SAFE_OPEN("testfile", O_RDWR | O_CREAT, 0700);
	SAFE_WRITE(SAFE_WRITE_ALL, fd, buf, SIZE);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = verify_read,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
