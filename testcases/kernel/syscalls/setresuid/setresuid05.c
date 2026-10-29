// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * Verify that after updating euid with setresuid(), any file creation
 * also gets the new euid as its owner user ID.
 */

#include <pwd.h>
#include <sys/stat.h>
#include "tst_test.h"
#include "compat_tst_16.h"

#define TEMP_FILE	"testfile"
static struct passwd *ltpuser;

static void setup(void)
{
	ltpuser = SAFE_GETPWNAM("nobody");
	UID16_CHECK(ltpuser->pw_uid, "setresuid");
}

static void run(void)
{
	struct stat buf;

	TST_EXP_PASS(SETRESUID(-1, ltpuser->pw_uid, -1));

	SAFE_TOUCH(TEMP_FILE, 0644, NULL);
	SAFE_STAT(TEMP_FILE, &buf);

	TST_EXP_EQ_LI(ltpuser->pw_uid, buf.st_uid);

	SAFE_UNLINK(TEMP_FILE);
	TST_EXP_PASS_SILENT(SETRESUID(-1, 0, -1));
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_root = 1,
	.needs_tmpdir = 1
};
