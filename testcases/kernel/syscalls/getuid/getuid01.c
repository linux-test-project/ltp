// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 *    AUTHOR           : William Roske
 *    CO-PILOT         : Dave Fenner
 */

/*\
 * [Description]
 *
 * Check the basic functionality of the getuid() system call.
 */

#include "tst_test.h"
#include "compat_tst_16.h"

static void verify_getuid(void)
{
	TST_EXP_POSITIVE(GETUID(), "getuid()");
}

static struct tst_test test = {
	.test_all = verify_getuid,
};
