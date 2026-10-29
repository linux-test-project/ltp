//SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) Linux Test Project, 2003-2023
 * Ported by Wayne Boyer
 */

/*\
 * Check that geteuid() return value matches value from /proc/self/status.
 */

#include "tst_test.h"
#include "compat_tst_16.h"

static void verify_geteuid(void)
{
	long uid[4];

	TST_EXP_POSITIVE(GETEUID(), "geteuid()");

	if (!TST_PASS)
		return;

	SAFE_FILE_LINES_SCANF("/proc/self/status", "Uid: %ld %ld %ld %ld",
			      &uid[0], &uid[1], &uid[2], &uid[3]);

	TST_EXP_EXPR(TST_RET == uid[1],
		     "geteuid() ret %ld == /proc/self/status EUID: %ld",
		     TST_RET, uid[1]);
}

static struct tst_test test = {
	.test_all = verify_geteuid,
};
