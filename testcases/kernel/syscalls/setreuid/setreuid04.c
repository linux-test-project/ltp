// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Ported by John George
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that root user can change the real and effective uid to an
 * unprivileged user.
 */

#include <pwd.h>
#include "tst_test.h"
#include "compat_tst_16.h"

static uid_t root_uid, nobody_uid;

static void setup(void)
{
	struct passwd *nobody;

	root_uid = getuid();
	nobody = SAFE_GETPWNAM("nobody");
	nobody_uid = nobody->pw_uid;

	UID16_CHECK(nobody_uid, setreuid);
	UID16_CHECK(root_uid, setreuid);
}

static void run(void)
{
	if (!SAFE_FORK()) {
		TST_EXP_PASS(SETREUID(nobody_uid, nobody_uid));

		TST_EXP_EQ_LI(GETUID(), nobody_uid);
		TST_EXP_EQ_LI(GETEUID(), nobody_uid);
	}
	tst_reap_children();
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_root = 1,
	.forks_child = 1
};
