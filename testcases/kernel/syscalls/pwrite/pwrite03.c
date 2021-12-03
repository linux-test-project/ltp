// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Carlo Marcelo Arenas Belon <carlo@gmail.com>
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * [Description]
 *
 * Tests for a special case NULL buffer with size 0 is expected to return 0.
 */

#include <errno.h>
#include "tst_test.h"

static int fd;

static void verify_pwrite(void)
{
	TEST(pwrite(fd, NULL, 0, 0));

	if (TST_RET != 0) {
		tst_res(TFAIL | TTERRNO,
			"pwrite() should have succeeded with ret=0");
		return;
	}

	tst_res(TPASS, "pwrite(fd, NULL, 0) == 0");
}

static void setup(void)
{
	fd = SAFE_OPEN("test_file", O_RDWR | O_CREAT, 0700);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_pwrite,
	.needs_tmpdir = 1,
};
