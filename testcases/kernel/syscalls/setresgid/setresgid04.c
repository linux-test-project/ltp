// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Copyright (c) Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that setresgid() syscall always sets the file system GID to the same
 * value as the new effective GID.
 */

#define _GNU_SOURCE

#include <pwd.h>
#include "tst_test.h"
#include "compat_tst_16.h"

static struct passwd *ltpuser;

static void run(void)
{
	struct stat buf;

	TST_EXP_PASS(SETRESGID(-1, ltpuser->pw_gid, -1));

	SAFE_TOUCH("test_file", 0644, NULL);
	SAFE_STAT("test_file", &buf);

	TST_EXP_EQ_LI(ltpuser->pw_gid, buf.st_gid);
}

static void setup(void)
{
	ltpuser = SAFE_GETPWNAM("nobody");

	GID16_CHECK(ltpuser->pw_gid, "setresgid");
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.needs_root = 1,
	.needs_tmpdir = 1,
};
