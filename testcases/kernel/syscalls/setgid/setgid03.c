// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  Ported by Wayne Boyer
 */

/*\
 * [Algorithm]
 *
 * As a root sets current group id to nobody and expects success.
 */

#include <pwd.h>
#include "tst_test.h"
#include <compat_tst_16.h>

static struct passwd *nobody;

static void run(void)
{
	TST_EXP_PASS(SETGID(nobody->pw_gid));

	if (getgid() != nobody->pw_gid)
		tst_res(TFAIL, "setgid failed to set gid to nobody gid");
	else
		tst_res(TPASS, "functionality of getgid() is correct");
}

static void setup(void)
{
	nobody = SAFE_GETPWNAM("nobody");
	GID16_CHECK(nobody->pw_gid, setgid);
}

static struct tst_test test = {
	.needs_root = 1,
	.setup = setup,
	.test_all = run,
};
