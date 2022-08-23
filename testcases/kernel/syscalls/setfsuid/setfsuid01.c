// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Ported by Wayne Boyer
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that setfsuid() correctly updates the filesystem user ID
 * to the value given in fsuid argument.
 */

#include <pwd.h>
#include "tst_test.h"
#include "compat_tst_16.h"

static uid_t nobody_uid;

static void setup(void)
{
	struct passwd *nobody;

	nobody = SAFE_GETPWNAM("nobody");
	nobody_uid = nobody->pw_uid;
}

static void run(void)
{
	uid_t uid;

	uid = geteuid();
	UID16_CHECK(uid, setfsuid);

	SAFE_SETEUID(0);
	TST_EXP_VAL(setfsuid(nobody_uid), uid, "setfsuid(%d)", nobody_uid);
	TST_EXP_VAL(setfsuid(-1), nobody_uid);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_root = 1
};
