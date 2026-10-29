// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Copyright (c) Linux Test Project, 2014-2025
 */

/*\
 * Verify that fchownat() will operate on symbolic links when
 * AT_SYMLINK_NOFOLLOW is used.
 */

#define _GNU_SOURCE
#include "tst_test.h"

#define TESTFILE	"testfile"
#define TESTFILE_LINK	"testfile_link"

static uid_t set_uid = 1000;
static gid_t set_gid = 1000;

static void setup(void)
{
	struct stat c_buf, l_buf;

	SAFE_TOUCH(TESTFILE, 0600, NULL);
	SAFE_SYMLINK(TESTFILE, TESTFILE_LINK);
	SAFE_STAT(TESTFILE_LINK, &c_buf);
	SAFE_LSTAT(TESTFILE_LINK, &l_buf);

	if (l_buf.st_uid == set_uid || l_buf.st_gid == set_gid) {
		tst_brk(TBROK,
			"uid link(%d) == set(%d) or gid link(%d) == set(%d)",
			l_buf.st_uid, set_uid, l_buf.st_gid, set_gid);
	}
}

static void run(void)
{
	struct stat c_buf, l_buf;

	TST_EXP_PASS(fchownat(AT_FDCWD, TESTFILE_LINK, set_uid, set_gid, AT_SYMLINK_NOFOLLOW),
		     "fchownat(%d, %s, %d, %d, %d)",
		     AT_FDCWD, TESTFILE_LINK, set_uid, set_gid, AT_SYMLINK_NOFOLLOW);

	SAFE_STAT(TESTFILE_LINK, &c_buf);
	SAFE_LSTAT(TESTFILE_LINK, &l_buf);

	TST_EXP_EXPR(c_buf.st_uid != set_uid && l_buf.st_uid == set_uid,
		     "fchownat() correctly operated on symlink user ID");
	TST_EXP_EXPR(c_buf.st_gid != set_gid && l_buf.st_gid == set_gid,
		     "fchownat() correctly operated on symlink group ID");
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.needs_root = 1,
	.test_all = run,
	.setup = setup,
};
