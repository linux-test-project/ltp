// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) Linux Test Project, 2003-2023
 *
 * Author: William Roske
 * CO-PILOT: Dave Fenner
 */

/*\
 * Check the basic functionality of the setgroups() system call.
 */

#include "tst_test.h"
#include "compat_tst_16.h"

static int len = NGROUPS;

static GID_T list[NGROUPS];

static void verify_setgroups(void)
{
	TST_EXP_POSITIVE(SETGROUPS(1, list), "setgroups()");
}

static void setup(void)
{
	len = GETGROUPS(NGROUPS, list);
	if (len < 0)
		tst_brk(TBROK | TERRNO, "getgroups() Failed");
}

static struct tst_test test = {
	.test_all = verify_setgroups,
	.setup = setup,
	.needs_root = 1,
};
