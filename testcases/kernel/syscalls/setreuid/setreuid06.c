// SPDX-License-Identifier: GPL-2.0-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Ported by John George
 * Copyright (c) 2023 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that setreuid(2) syscall fails with EPERM errno when the calling
 * process is not privileged and a change other than
 * (i) swapping the effective user ID with the real user ID, or
 * (ii) setting one to the value of the other or
 * (iii) setting the effective user ID to the value of the saved set-user-ID
 * was specified.
 */

#include <pwd.h>
#include "tst_test.h"
#include "tst_uid.h"
#include "compat_tst_16.h"

static struct passwd *ltpuser;
static uid_t other_uid;

static void setup(void)
{
	tst_get_uids(&other_uid, 0, 1);

	UID16_CHECK(other_uid, setreuid);

	ltpuser = SAFE_GETPWNAM("nobody");
	SAFE_SETUID(ltpuser->pw_uid);
}

static void run(void)
{

	TST_EXP_FAIL(SETREUID(-1, other_uid), EPERM,
				"setreuid(%d, %d)", -1, other_uid);
	TST_EXP_FAIL(SETREUID(other_uid, -1), EPERM,
				"setreuid(%d, %d)", other_uid, -1);
	TST_EXP_FAIL(SETREUID(other_uid, other_uid), EPERM,
				"setreuid(%d, %d)", other_uid, other_uid);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_root = 1
};
