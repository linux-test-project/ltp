// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that setegid() fails with EPERM when the calling process is not
 * privileged and egid does not match the current real group ID,
 * current effective group ID, or current saved set-group-ID.
 */

#include <pwd.h>
#include "tst_test.h"

static struct passwd *ltpuser;

static void setup(void)
{
	ltpuser = SAFE_GETPWNAM("nobody");
	SAFE_SETEUID(ltpuser->pw_uid);
}

static void setegid_verify(void)
{
	TST_EXP_FAIL(setegid(ltpuser->pw_gid),
				EPERM,
				"setegid(%d)",
				ltpuser->pw_gid);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = setegid_verify,
	.needs_root = 1
};
