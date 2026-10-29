// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) Linux Test Project, 2001-2023
 * Author: William Roske
 */

/*\
 * Check the basic functionality of the symlink() system call.
 */

#include "tst_test.h"

static char *fname, *symlnk;

static void verify_symlink(void)
{
	TST_EXP_POSITIVE(symlink(fname, symlnk), "symlink(%s, %s)",
			 fname, symlnk);

	if (TST_RET == -1)
		tst_res(TFAIL, "symlink(%s, %s) Failed", fname, symlnk);
	else
		SAFE_UNLINK(symlnk);
}

static void setup(void)
{
	fname = tst_aprintf("tfile_%d", getpid());

	symlnk = tst_aprintf("st_%d", getpid());
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.test_all = verify_symlink,
};
