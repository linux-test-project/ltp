// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *	Author: William Roske
 *	Co-pilot: Dave Fenner
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * Verify the basic functionality of setreuid(2) system call when executed
 * as non-root user.
 */

#include "tst_test.h"
#include "compat_tst_16.h"

static uid_t ruid, euid;

static void run(void)
{
	ruid = getuid();
	UID16_CHECK(ruid, setreuid);

	euid = geteuid();
	UID16_CHECK(euid, setreuid);

	TST_EXP_PASS(SETREUID(-1, -1));
	TST_EXP_PASS(SETREUID(-1, euid));
	TST_EXP_PASS(SETREUID(ruid, -1));
	TST_EXP_PASS(SETREUID(-1, ruid));
	TST_EXP_PASS(SETREUID(euid, -1));
	TST_EXP_PASS(SETREUID(euid, euid));
	TST_EXP_PASS(SETREUID(ruid, ruid));
}

static struct tst_test test = {
	.test_all = run
};
