// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * AUTHOR		: William Roske
 * CO-PILOT		: Dave Fenner
 */

/*\
 * Call getgid() and expects that the gid returned correctly.
 */

#include <pwd.h>
#include "tst_test.h"
#include "compat_tst_16.h"

static struct passwd *ltpuser;

static void run(void)
{
	TEST(GETGID());
	if (TST_RET != ltpuser->pw_gid)
		tst_res(TFAIL, "getgid failed, returned %ld", TST_RET);
	else
		tst_res(TPASS, "getgid returned as expectedly");
}

static void setup(void)
{
	ltpuser = SAFE_GETPWNAM("nobody");
	SAFE_SETGID(ltpuser->pw_gid);
}

static struct tst_test test = {
	.needs_root = 1,
	.test_all = run,
	.setup = setup,
};
