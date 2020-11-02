// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) 2020 SUSE LLC
 *
 * 03/30/1992 AUTHOR: William Roske CO-PILOT: Dave Fenner
 *
 */

#include "tst_test.h"

static int fd;

static void verify_dup(void)
{
	TEST(dup(fd));

	if (TST_RET < -1) {
		tst_res(TFAIL, "Invalid dup() return value %ld", TST_RET);
	} else if (TST_RET == -1) {
		tst_res(TFAIL | TERRNO, "dup(%d) Failed", fd);
	} else {
		tst_res(TPASS, "dup(%d) returned %ld", fd, TST_RET);
		SAFE_CLOSE(TST_RET);
	}
}

static void setup(void)
{
	fd = SAFE_OPEN("dupfile", O_RDWR | O_CREAT, 0700);
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
