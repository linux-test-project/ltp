// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 */

/*\
 * [Description]
 *
 * Verify that, chmod(2) will succeed to change the mode of a file or directory
 * and set the sticky bit on it if invoked by non-root (uid != 0)
 * process with the following constraints:
 *
 * - the process is the owner of the file or directory.
 * - the effective group ID or one of the supplementary group ID's of the
 *   process is equal to the group ID of the file or directory.
 */

#include <pwd.h>
#include "tst_test.h"

#define FILE_MODE   S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define DIR_MODE	S_IRWXU | S_IRWXG | S_IRWXO
#define PERMS		01777

#define TESTFILE	"testfile"
#define TESTDIR		"testdir_3"

static struct tcase {
	char *name;
	char *desc;
} tcases[] = {
	{TESTFILE, "verify permissions of file"},
	{TESTDIR, "verify permissions of directory"},
};

static void verify_chmod(unsigned int n)
{
	struct stat stat_buf;
	struct tcase *tc = &tcases[n];

	TST_EXP_PASS(chmod(tc->name, PERMS), "chmod(%s, %04o)",
		tc->name, PERMS);

	if (!TST_PASS)
		return;

	SAFE_STAT(tc->name, &stat_buf);

	if ((stat_buf.st_mode & PERMS) != PERMS) {
		tst_res(TFAIL, "stat(%s) mode=%04o",
			tc->name, stat_buf.st_mode);
	} else {
		tst_res(TPASS, "stat(%s) mode=%04o",
			tc->name, stat_buf.st_mode);
	}
}

static void setup(void)
{
	char nobody_uid[] = "nobody";
	struct passwd *ltpuser;

	ltpuser = SAFE_GETPWNAM(nobody_uid);
	SAFE_SETEUID(ltpuser->pw_uid);

	SAFE_TOUCH(TESTFILE, FILE_MODE, NULL);
	SAFE_MKDIR(TESTDIR, DIR_MODE);
}

static struct tst_test test = {
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_chmod,
	.needs_root = 1,
	.needs_tmpdir = 1,
};
