// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2001
 */

/*
 * DESCRIPTION
 *	fchdir01 - create a directory and cd into it.
 */

#include "tst_test.h"

static int fd;
static const char *TEST_DIR = "alpha";

#define MODES	S_IRWXU

static void verify_fchdir(void)
{
	TST_EXP_PASS(fchdir(fd));
}

static void setup(void)
{
	SAFE_MKDIR(TEST_DIR, MODES);
	fd = SAFE_OPEN(TEST_DIR, O_RDONLY);
}

static void cleanup(void)
{
	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = verify_fchdir,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
