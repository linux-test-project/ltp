// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * Test Description:
 *  Verify that, fchmod() will succeed to change the mode of a file/directory
 *  set the sticky bit on it if invoked by root (uid = 0) process with
 *  the following constraints,
 *	- the process is not the owner of the file/directory.
 *	- the effective group ID or one of the supplementary group ID's of the
 *	  process is equal to the group ID of the file/directory.
 *
 * Expected Result:
 *  fchmod() should return value 0 on success and succeeds to set sticky bit
 *  on the specified file.
 */

#include <pwd.h>
#include <grp.h>
#include <errno.h>

#include "tst_test.h"
#include "fchmod.h"

static int fd;

static void verify_fchmod(void)
{
	struct stat stat_buf;
	mode_t file_mode;

	TEST(fchmod(fd, PERMS));
	if (TST_RET == -1)
		tst_res(TFAIL | TTERRNO, "fchmod() failed unexpectly");

	SAFE_FSTAT(fd, &stat_buf);
	file_mode = stat_buf.st_mode;

	if ((file_mode & ~S_IFREG) != PERMS) {
		tst_res(TFAIL, "%s: Incorrect modes 0%03o, Expected 0%03o",
			TESTFILE, file_mode, PERMS);
	} else {
		tst_res(TPASS, "Functionality of fchmod(%d, %#o) Successful",
			fd, PERMS);
	}
}

static void setup(void)
{
	struct passwd *ltpuser;
	struct group *ltpgroup;

	ltpuser = SAFE_GETPWNAM("nobody");
	ltpgroup = SAFE_GETGRNAM_FALLBACK("users", "daemon");

	fd = SAFE_OPEN(TESTFILE, O_RDWR | O_CREAT, FILE_MODE);
	SAFE_CHOWN(TESTFILE, ltpuser->pw_uid, ltpgroup->gr_gid);
	SAFE_SETGID(ltpgroup->gr_gid);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = verify_fchmod,
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
