// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2001-2022
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 */

/*\
 * [Description]
 *
 * Testcase to check open() with O_RDWR | O_CREAT.
 */

#include "tst_test.h"

#define TEST_FILE "testfile"

static void verify_open(void)
{
	TST_EXP_FD(open(TEST_FILE, O_RDWR | O_CREAT, 0700));
	SAFE_CLOSE(TST_RET);
	SAFE_UNLINK(TEST_FILE);
}



static struct tst_test test = {
	.needs_tmpdir = 1,
	.test_all = verify_open,
};
