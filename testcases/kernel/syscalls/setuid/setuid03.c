// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 */

/* DESCRIPTION
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
	TEST(SETUID(ROOT_USER));
	if (TST_RET != -1) {
		tst_res(TFAIL | TTERRNO, "setuid() succeeded unexpectedly");
		return;
	}

	if (TST_ERR == EPERM)
		tst_res(TPASS, "setuid() returned errno EPERM");
	else
		tst_res(TFAIL | TTERRNO, "setuid() returned unexpected errno");
}

static void setup(void)
{
	struct passwd *pw;
	uid_t uid;

	pw = SAFE_GETPWNAM("nobody");
	uid = pw->pw_uid;

	if (SETUID(uid) == -1) {
		tst_brk(TBROK,
			"setuid() failed to set the effective uid to %d", uid);
	}

	umask(0);
}

static struct tst_test test = {
	.setup = setup,
	.needs_root = 1,
	.test_all =  verify_setuid,
};
