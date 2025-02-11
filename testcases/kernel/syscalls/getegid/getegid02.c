// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *   William Roske, Dave Fenner
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * This test checks if getegid() returns the same effective group given by
 * passwd entry via getpwuid().
 */

#include <pwd.h>

#include "tst_test.h"
#include "compat_tst_16.h"

static void run(void)
{
	uid_t euid;
	gid_t egid;
	struct passwd *pwent;

	UID16_CHECK((euid = geteuid()), "geteuid");

	pwent = getpwuid(euid);
	if (!pwent)
		tst_brk(TBROK | TERRNO, "getpwuid() error");

	GID16_CHECK((egid = getegid()), "getegid");

	TST_EXP_EQ_LI(pwent->pw_gid, egid);
}

static struct tst_test test = {
	.test_all = run,
};
