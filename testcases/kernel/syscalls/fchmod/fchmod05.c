// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Author: Wayne Boyer
 *
 * Test Description:
 *  Verify that, fchmod(2) will succeed to change the mode of a directory
 *  but fails to set the setgid bit on it if invoked by non-root (uid != 0)
 *  process with the following constraints,
 *	- the process is the owner of the directory.
 *	- the effective group ID or one of the supplementary group ID's of the
 *	  process is not equal to the group ID of the directory.
 *
 * Expected Result:
 *  fchmod() should return value 0 on success and though succeeds to change
 *  the mode of a directory but fails to set setgid bit on it.
 */

#include <pwd.h>
#include <errno.h>

#include "tst_test.h"
#include "tst_uid.h"
#include "fchmod.h"

#define PERMS_DIR	043777

static int fd;

static void verify_fchmod(void)
{
	struct stat stat_buf;
	mode_t dir_mode;

	TEST(fchmod(fd, PERMS_DIR));
	if (TST_RET == -1)
		tst_res(TFAIL | TTERRNO, "fchmod() failed unexpectly");

	SAFE_FSTAT(fd, &stat_buf);
	dir_mode = stat_buf.st_mode;

	if ((PERMS_DIR & ~S_ISGID) != dir_mode) {
		tst_res(TFAIL, "%s: Incorrect modes 0%03o, Expected 0%03o",
			TESTDIR, dir_mode & ~S_ISGID, PERMS_DIR);
	} else {
		tst_res(TPASS, "Functionality of fchmod(%d, %#o) successful",
			fd, PERMS_DIR);
	}
}

static void setup(void)
{
	struct passwd *ltpuser;
	gid_t free_gid;

	ltpuser = SAFE_GETPWNAM("nobody");
	free_gid = tst_get_free_gid(ltpuser->pw_gid);

	SAFE_MKDIR(TESTDIR, DIR_MODE);

	if (setgroups(1, &ltpuser->pw_gid) == -1) {
		tst_brk(TBROK, "Couldn't change supplementary group Id: %s",
			tst_strerrno(TST_ERR));
	}

	SAFE_CHOWN(TESTDIR, ltpuser->pw_uid, free_gid);

	SAFE_SETEGID(ltpuser->pw_gid);
	SAFE_SETEUID(ltpuser->pw_uid);

	fd = SAFE_OPEN(TESTDIR, O_RDONLY);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);

	SAFE_SETEGID(0);
	SAFE_SETEUID(0);
}

static struct tst_test test = {
	.test_all = verify_fchmod,
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
