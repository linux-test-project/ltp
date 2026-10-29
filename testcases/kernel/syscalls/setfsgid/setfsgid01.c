// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Ported by Wayne Boyer
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * Verify that setfsgid() correctly updates the filesystem group ID
 * to the value given in fsgid argument.
 */

#include <pwd.h>
#include <unistd.h>

#include "tst_test.h"
#include "compat_tst_16.h"

static gid_t nobody_gid;

static void setup(void)
{
	struct passwd *nobody;

	nobody = SAFE_GETPWNAM("nobody");
	nobody_gid = nobody->pw_gid;
}

static void run(void)
{
	gid_t gid;

	gid = getegid();
	GID16_CHECK(gid, setfsgid);

	SAFE_SETEUID(0);
	TST_EXP_VAL(SETFSGID(nobody_gid), gid);
	TST_EXP_VAL(SETFSGID(-1), nobody_gid);
	TST_EXP_VAL_SILENT(SETFSGID(gid), nobody_gid);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_root = 1
};
