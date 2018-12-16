/// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2001
 */

/*
 * Test Name: chmod05
 *
 * Test Description:
 *  Verify that, chmod(2) will succeed to change the mode of a directory
 *  but fails to set the setgid bit on it if invoked by non-root (uid != 0)
 *  process with the following constraints,
 *	- the process is the owner of the directory.
 *	- the effective group ID or one of the supplementary group ID's of the
 *	  process is not equal to the group ID of the directory.
 *
 * Expected Result:
 *  chmod() should return value 0 on success and though succeeds to change
 *  the mode of a directory but fails to set setgid bit on it.
 *
 */

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>

#include "tst_test.h"

#define MODE_RWX	(mode_t)(S_IRWXU | S_IRWXG | S_IRWXO)
#define DIR_MODE	(mode_t)(S_ISVTX | S_ISGID | S_IFDIR)
#define PERMS		(mode_t)(MODE_RWX | DIR_MODE)
#define TESTDIR		"testdir"

static void test_chmod(void)
{
	struct stat stat_buf;
	mode_t dir_mode;

	TEST(chmod(TESTDIR, PERMS));
	if (TST_RET == -1) {
		tst_res(TFAIL, "chmod(%s, %#o) failed", TESTDIR, PERMS);
		return;
	}

	SAFE_STAT(TESTDIR, &stat_buf);
	dir_mode = stat_buf.st_mode;
	if ((PERMS & ~S_ISGID) != dir_mode) {
		tst_res(TFAIL, "%s: Incorrect modes 0%03o, "
				"Expected 0%03o", TESTDIR, dir_mode,
				PERMS & ~S_ISGID);
	} else {
		tst_res(TPASS, "Functionality of chmod(%s, %#o) successful",
				TESTDIR, PERMS);
	}
}

static void setup(void)
{
	struct passwd *nobody_u;
	struct group *bin_gr;

	nobody_u = SAFE_GETPWNAM("nobody");
	bin_gr = SAFE_GETGRNAM("bin");

	/*
	 * Create a test directory under temporary directory with specified
	 * mode permissions and change the gid of test directory to nobody's
	 * gid.
	 */
	SAFE_MKDIR(TESTDIR, MODE_RWX);
	if (setgroups(1, &nobody_u->pw_gid) == -1)
		tst_brk(TBROK | TERRNO, "setgroups to nobody's gid failed");

	SAFE_CHOWN(TESTDIR, nobody_u->pw_uid, bin_gr->gr_gid);

	/* change to nobody:nobody */
	SAFE_SETEGID(nobody_u->pw_gid);
	SAFE_SETEUID(nobody_u->pw_uid);
}

static struct tst_test test = {
	.needs_root	= 1,
	.needs_tmpdir	= 1,
	.setup		= setup,
	.test_all	= test_chmod,
};
