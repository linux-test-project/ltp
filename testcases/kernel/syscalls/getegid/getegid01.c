// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *   William Roske, Dave Fenner
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * This test checks if getegid() returns the effective group id.
 */

#include "tst_test.h"
#include "compat_tst_16.h"

static void run(void)
{
	gid_t gid, st_egid;

	SAFE_FILE_LINES_SCANF("/proc/self/status", "Gid: %*d %d", &st_egid);
	gid = getegid();

	if (GID_SIZE_CHECK(st_egid))
		TST_EXP_EQ_LI(gid, st_egid);
	else
		tst_res(TPASS, "getegid() passed");
}

static struct tst_test test = {
	.test_all = run,
};
