// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Carlo Marcelo Arenas Belon <carlo@gmail.com>
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2003-2023
 */

/*\
 * Tests for a special case NULL buffer with size 0 is expected to return 0.
 */

#include "tst_test.h"

static int fd;

static void verify_write(void)
{
	TST_EXP_POSITIVE(write(fd, NULL, 0));

	TST_EXP_EXPR(TST_RET == 0, "write(fd, NULL, %ld) == %d", TST_RET, 0);
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
	.test_all = verify_write,
	.needs_tmpdir = 1,
};
