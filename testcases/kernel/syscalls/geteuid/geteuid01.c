//SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) Linux Test Project, 2003-2023
 *
 * Author: William Roske
 * CO-PILOT: Dave Fenner
 */

/*\
 * Check the basic functionality of the geteuid() system call.
 */

#include "tst_test.h"
#include "compat_tst_16.h"

static void verify_geteuid(void)
{
	TST_EXP_POSITIVE(GETEUID(), "geteuid()");
}

static struct tst_test test = {
	.test_all = verify_geteuid,
};
