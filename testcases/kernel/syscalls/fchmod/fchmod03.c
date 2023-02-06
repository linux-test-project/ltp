// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2001
 */

/*\
 * [Description]
 *
 * Verify that, fchmod(2) will succeed to change the mode of a file
 * and set the sticky bit on it if invoked by non-root (uid != 0)
 * process with the following constraints:
 *
 * - the process is the owner of the file
 * - the effective group ID or one of the supplementary group ID's of the
 *   process is equal to the group ID of the file
 */

#include <pwd.h>
#include "fchmod.h"
#include "tst_test.h"

static int fd;
static const char nobody_uid[] = "nobody";
static struct passwd *ltpuser;

static void verify_fchmod(void)
{
	struct stat stat_buf;
	mode_t file_mode;

	TST_EXP_PASS_SILENT(fchmod(fd, PERMS));

	if (fstat(fd, &stat_buf) == -1)
		tst_brk(TFAIL | TERRNO, "fstat failed");
	file_mode = stat_buf.st_mode;

	if ((file_mode & PERMS) != PERMS)
		tst_res(TFAIL, "%s: Incorrect modes 0%3o, "
			"Expected 0777", TESTFILE, file_mode);
	else
		tst_res(TPASS, "Functionality of fchmod(%d, "
			"%#o) successful", fd, PERMS);
}

static void setup(void)
{
	ltpuser = SAFE_GETPWNAM(nobody_uid);

	SAFE_SETEUID(ltpuser->pw_uid);

	fd = SAFE_OPEN(TESTFILE, O_RDWR | O_CREAT, FILE_MODE);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = verify_fchmod,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.needs_root = 1,
};
