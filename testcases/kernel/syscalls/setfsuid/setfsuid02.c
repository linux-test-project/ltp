// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) International Business Machines  Corp., 2001
 * Ported by Wayne Boyer
 * Adapted by Dustin Kirkland (k1rkland@us.ibm.com)
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * Verify that setfsuid() syscall fails if an invalid fsuid is given.
 */

#include <pwd.h>
#include "tst_test.h"
#include "compat_tst_16.h"

static void run(void)
{
	uid_t invalid_uid, current_uid;

	current_uid = geteuid();
	invalid_uid = (UID_T)-1;

	UID16_CHECK(invalid_uid, setfsuid);

	TST_EXP_VAL_SILENT(SETFSUID(invalid_uid), (long)current_uid);
	TST_EXP_VAL(SETFSUID(invalid_uid), (long)current_uid,
		    "setfsuid(invalid_fsuid) test for expected failure:");
}

static struct tst_test test = {
	.test_all = run
};
