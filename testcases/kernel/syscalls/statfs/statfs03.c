// SPDX-License-Identifier: GPL-2.0-or-later

/*
 *   Copyright (C) Bull S.A. 2001
 *   Copyright (c) International Business Machines  Corp., 2001
 *		05/2002 Ported by Jacky Malcles
 *   Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that statfs(2) fails with errno EACCES when search permission
 * is denied for a component of the path prefix of path.
 */

#include <pwd.h>
#include "tst_test.h"

#define TEMP_DIR "testdir"
#define TEMP_DIR2 TEMP_DIR"/subdir"

static void setup(void)
{
	struct passwd *ltpuser;

	SAFE_MKDIR(TEMP_DIR, 0444);
	SAFE_MKDIR(TEMP_DIR2, 0444);

	ltpuser = SAFE_GETPWNAM("nobody");
	SAFE_SETEUID(ltpuser->pw_uid);
}

static void run(void)
{
	struct statfs buf;

	TST_EXP_FAIL(statfs(TEMP_DIR2, &buf), EACCES);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_tmpdir = 1,
	.needs_root = 1
};
