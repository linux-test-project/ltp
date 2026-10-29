// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  Ported by Wayne Boyer
 */

/*\
 * Test if setgid() system call sets errno to EPERM correctly.
 *
 * [Algorithm]
 *
 * Call setgid() to set the gid to that of root. Run this test as
 * nobody, and expect to get EPERM.
 */

#include <pwd.h>
#include "tst_test.h"
#include "compat_tst_16.h"

static void run(void)
{
	struct passwd *rootpwent;

	rootpwent = SAFE_GETPWNAM("root");

	GID16_CHECK(rootpwent->pw_gid, setgid);

	TST_EXP_FAIL(SETGID(rootpwent->pw_gid), EPERM);
}

static void setup(void)
{
	struct passwd *nobody = SAFE_GETPWNAM("nobody");

	SAFE_SETGID(nobody->pw_gid);
	SAFE_SETUID(nobody->pw_uid);
}

static struct tst_test test = {
	.needs_root = 1,
	.setup = setup,
	.test_all = run,
};
