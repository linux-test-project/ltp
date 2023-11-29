// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) Linux Test Project, 2001-2023
 * Author: John George
 */

/*\
 * [Description]
 *
 * Check that a symbolic link may point to an existing file or
 * to a nonexistent one.
 */

#include <stdlib.h>
#include <stdio.h>
#include "tst_test.h"

#define TESTFILE    "testfile"
#define NONFILE     "noexistfile"
#define SYMFILE     "slink_file"

static char *testfile;
static char *nonfile;

static struct tcase {
	char **srcfile;
} tcases[] = {
	{&testfile},
	{&nonfile},
};

static void setup(void)
{
	SAFE_TOUCH(TESTFILE, 0644, NULL);
}

static void verify_symlink(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	struct stat stat_buf;

	TST_EXP_PASS(symlink(*tc->srcfile, SYMFILE));

	SAFE_LSTAT(SYMFILE, &stat_buf);

	if (!S_ISLNK(stat_buf.st_mode))
		tst_res(TFAIL, "symlink of %s doesn't exist", *tc->srcfile);

	SAFE_UNLINK(SYMFILE);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.test = verify_symlink,
	.bufs = (struct tst_buffers []) {
		{&testfile, .str = TESTFILE},
		{&nonfile, .str = NONFILE},
		{},
	},
	.needs_tmpdir = 1,
};
