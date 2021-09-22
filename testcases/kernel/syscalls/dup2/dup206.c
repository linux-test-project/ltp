// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*\
 * [Description]
 *
 * If oldfd is a valid file descriptor, and newfd has the same value as oldfd,
 * then dup2() does nothing, and returns newfd.
 */

#include <stdio.h>
#include <unistd.h>
#include "tst_test.h"

static int fd = -1;

static void verify_dup2(void)
{
	TST_EXP_FD_SILENT(dup2(fd, fd), "dup2(%d, %d)", fd, fd);

	if (TST_RET != fd) {
		tst_res(TFAIL, "dup2(%d, %d) returns wrong newfd(%ld)", fd, fd, TST_RET);
		SAFE_CLOSE(TST_RET);
		return;
	}
	tst_res(TPASS, "dup2(%d, %d) returns newfd(%d)", fd, fd, fd);
}

static void setup(void)
{
	fd = SAFE_OPEN("testfile", O_RDWR | O_CREAT, 0666);
}

static void cleanup(void)
{
	if (fd > -1)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_dup2,
};
