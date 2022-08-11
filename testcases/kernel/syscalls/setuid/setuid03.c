// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Copyright (c) Linux Test Project, 2009-2022
 */

/*\
 * [Description]
 *
 * This test will switch to nobody user for correct error code collection.
 * Verify setuid returns errno EPERM when it switches to root_user.
 */

#include <errno.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include "tst_test.h"
#include "compat_tst_16.h"

#define ROOT_USER	0

static void verify_setuid(void)
{
	TST_EXP_FAIL(SETUID(ROOT_USER), EPERM);
}

static void setup(void)
{
	struct passwd *pw;
	uid_t uid;

	pw = SAFE_GETPWNAM("nobody");
	uid = pw->pw_uid;

	SAFE_SETUID(uid);

	umask(0);
}

static struct tst_test test = {
	.setup = setup,
	.needs_root = 1,
	.test_all =  verify_setuid,
};
