// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * AUTHOR: William Roske
 * CO-PILOT: Dave Fenner
 * Copyright (c) 2021 Xie Ziyao <xieziyao@huawei.com>
 */

/*\
 * Basic test for chown(). Calls chown() on a file and expects it to pass.
 */

#include "tst_test.h"
#include "compat_tst_16.h"

#define FILENAME "chown01_testfile"

static int uid, gid;

static void run(void)
{
	TST_EXP_PASS(CHOWN(FILENAME, uid, gid), "chown(%s,%d,%d)",
		     FILENAME, uid, gid);
}

static void setup(void)
{
	UID16_CHECK((uid = geteuid()), "chown");
	GID16_CHECK((gid = getegid()), "chown");
	SAFE_FILE_PRINTF(FILENAME, "davef");
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.test_all = run,
};

