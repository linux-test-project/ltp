// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) Linux Test Project, 2003-2024
 */

/*\
 * [Description]
 *
 * Check the basic functionality of the fpathconf(2) system call.
 */

#include "tst_test.h"

static struct tcase {
	char *name;
	int value;
} test_cases[] = {
	{"_PC_MAX_CANON", _PC_MAX_CANON},
	{"_PC_MAX_INPUT", _PC_MAX_INPUT},
	{"_PC_VDISABLE", _PC_VDISABLE},
	{"_PC_LINK_MAX", _PC_LINK_MAX},
	{"_PC_NAME_MAX", _PC_NAME_MAX},
	{"_PC_PATH_MAX", _PC_PATH_MAX},
	{"_PC_PIPE_BUF", _PC_PIPE_BUF},
	{"_PC_CHOWN_RESTRICTED", _PC_CHOWN_RESTRICTED},
	{"_PC_NO_TRUNC", _PC_NO_TRUNC},
};

static int fd;

static void verify_fpathconf(unsigned int n)
{
	struct tcase *tc = &test_cases[n];

	TST_EXP_POSITIVE(fpathconf(fd, tc->value));
}

static void setup(void)
{
	fd = SAFE_OPEN("fpafile01", O_RDWR | O_CREAT, 0700);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.test  = verify_fpathconf,
	.tcnt = ARRAY_SIZE(test_cases),
	.setup = setup,
	.cleanup = cleanup,
};
