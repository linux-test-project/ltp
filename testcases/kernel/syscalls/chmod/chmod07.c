// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2001
 */

/*
 * Test Name: chmod07
 *
 * Test Description:
 *  Verify that, chmod(2) will succeed to change the mode of a file/directory
 *  and sets the sticky bit on it if invoked by root (uid = 0) process with
 *  the following constraints,
 *	- the process is not the owner of the file/directory.
 *	- the effective group ID or one of the supplementary group ID's of the
 *	  process is equal to the group ID of the file/directory.
 *
 * Expected Result:
 *  chmod() should return value 0 on success and succeeds to set sticky bit
 *  on the specified file.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <grp.h>
#include <pwd.h>

#include "tst_test.h"

#define FILE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define PERMS		01777	/* Permissions with sticky bit set. */
#define TESTFILE	"testfile"

void test_chmod(void)
{
	struct stat stat_buf;

	/*
	 * Call chmod(2) with specified mode argument
	 * (sticky-bit set) on testfile.
	 */
	TEST(chmod(TESTFILE, PERMS));
	if (TST_RET == -1) {
		tst_brk(TFAIL | TTERRNO, "chmod(%s, %#o) failed",
				TESTFILE, PERMS);
	}

	if (stat(TESTFILE, &stat_buf) == -1) {
		tst_brk(TFAIL | TTERRNO, "stat failed");
	}

	/* Check for expected mode permissions */
	if ((stat_buf.st_mode & PERMS) == PERMS) {
		tst_res(TPASS, "Functionality of chmod(%s, %#o) successful",
				TESTFILE, PERMS);
	} else {
		tst_res(TFAIL, "%s: Incorrect modes 0%03o; expected 0%03o",
				TESTFILE, stat_buf.st_mode, PERMS);
	}
}

void setup(void)
{
	struct passwd *ltpuser;
	struct group *ltpgroup;
	int fd;
	gid_t group1_gid;
	uid_t user1_uid;

	ltpuser = SAFE_GETPWNAM("nobody");
	user1_uid = ltpuser->pw_uid;

	ltpgroup = SAFE_GETGRNAM_FALLBACK("users", "daemon");
	group1_gid = ltpgroup->gr_gid;

	fd = SAFE_OPEN(TESTFILE, O_RDWR | O_CREAT, FILE_MODE);
	SAFE_CLOSE(fd);
	SAFE_CHOWN(TESTFILE, user1_uid, group1_gid);
	SAFE_SETGID(group1_gid);
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_tmpdir = 1,
	.setup = setup,
	.test_all = test_chmod,
};
