// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 */

/*  DESCRIPTION
 *  This test will verify that new directory created by mkdir(2) inherites
 *  the group ID from the parent directory and S_ISGID bit, if the S_ISGID
 *  bit is set in the parent directory.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include "tst_test.h"
#include "tst_uid.h"

#define TESTDIR1	"testdir1"
#define TESTDIR2	"testdir1/testdir2"

static gid_t free_gid;

static void verify_mkdir(void)
{
	struct stat statbuf;
	int fail = 0;

	SAFE_MKDIR(TESTDIR2, 0777);
	SAFE_STAT(TESTDIR2, &statbuf);

	if (statbuf.st_gid != free_gid) {
		tst_res(TFAIL,
			"New dir FAILED to inherit GID: has %d, expected %d",
			statbuf.st_gid, free_gid);
		fail = 1;
	}

	if (!(statbuf.st_mode & S_ISGID)) {
		tst_res(TFAIL, "New dir FAILED to inherit S_ISGID");
		fail = 1;
	}

	if (!fail)
		tst_res(TPASS, "New dir inherited GID and S_ISGID");

	SAFE_RMDIR(TESTDIR2);
}


static void setup(void)
{
	struct passwd *pw = SAFE_GETPWNAM("nobody");

	free_gid = tst_get_free_gid(pw->pw_gid);

	umask(0);
	SAFE_MKDIR(TESTDIR1, 0777);
	SAFE_CHMOD(TESTDIR1, 0777 | S_ISGID);
	SAFE_CHOWN(TESTDIR1, getuid(), free_gid);

	SAFE_SETREGID(pw->pw_gid, pw->pw_gid);
	SAFE_SETREUID(pw->pw_uid, pw->pw_uid);
}

static struct tst_test test = {
	.setup = setup,
	.needs_tmpdir = 1,
	.needs_root = 1,
	.test_all = verify_mkdir,
};
