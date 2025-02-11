/*
 * Copyright (C) International Business Machines  Corp., 2001
 * Ported by Wayne Boyer
 * Adapted by Dustin Kirkland (k1rkland@us.ibm.com)
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * Verify that setfsuid() correctly updates the filesystem uid
 * when caller is a non-root user and provided fsuid matches
 * caller's real user ID.
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
	uid_t ruid, euid, suid;

	SAFE_GETRESUID(&ruid, &euid, &suid);
	SAFE_SETEUID(nobody_uid);
	UID16_CHECK(ruid, setfsuid);

	TST_EXP_VAL_SILENT(SETFSUID(ruid), nobody_uid);
	TST_EXP_VAL(SETFSUID(-1), ruid, "setfsuid(fsuid) by non-root user:");
}

static struct tst_test test = {
	.needs_root = 1,
	.setup = setup,
	.test_all = run
};
