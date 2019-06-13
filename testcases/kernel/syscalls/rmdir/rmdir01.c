// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 */

/* DESCRIPTION
 *   This test will verify that rmdir(2) syscall basic functionality.
 *   verify rmdir(2) returns a value of 0 and the directory being removed.
 */
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "tst_test.h"

#define TESTDIR "testdir"

static void verify_rmdir(void)
{
	struct stat buf;

	SAFE_MKDIR(TESTDIR, 0777);

	TEST(rmdir(TESTDIR));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "rmdir(%s) failed", TESTDIR);
		return;
	}

	if (!stat(TESTDIR, &buf))
		tst_res(TFAIL, "rmdir(%s) failed", TESTDIR);
	else
		tst_res(TPASS, "rmdir(%s) success", TESTDIR);
}

static struct tst_test test = {
	.test_all = verify_rmdir,
	.needs_tmpdir = 1,
};

