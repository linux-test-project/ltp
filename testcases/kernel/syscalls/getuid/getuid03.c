// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  Ported by Wayne Boyer
 */

/*\
 * Check that getuid() return value matches value from /proc/self/status.
 */

#include "tst_test.h"
#include "compat_tst_16.h"

static void verify_getuid(void)
{
	long uid;

	TST_EXP_POSITIVE(GETUID(), "getuid()");

	if (!TST_PASS)
		return;

	SAFE_FILE_LINES_SCANF("/proc/self/status", "Uid: %ld", &uid);

	if (TST_RET != uid) {
		tst_res(TFAIL,
			"getuid() ret %ld != /proc/self/status Uid: %ld",
			TST_RET, uid);
	} else {
		tst_res(TPASS,
			"getuid() ret == /proc/self/status Uid: %ld", uid);
	}
}

static struct tst_test test = {
	.test_all = verify_getuid,
};
