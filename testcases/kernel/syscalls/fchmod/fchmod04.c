// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2001
 */

/*\
 * [Description]
 *
 * Verify that, fchmod(2) will succeed to change the mode of a directory
 * and set the sticky bit on it if invoked by non-root (uid != 0) process
 * with the following constraints:
 *
 * - the process is the owner of the directory
 * - the effective group ID or one of the supplementary group ID's of the
 *   process is equal to the group ID of the directory
 */

#include <pwd.h>
#include <stdio.h>
#include "fchmod.h"
#include "tst_test.h"

static int fd;
static const char nobody_uid[] = "nobody";

static void verify_fchmod(void)
{
	struct stat stat_buf;
	mode_t dir_mode;

	TST_EXP_PASS_SILENT(fchmod(fd, PERMS));

	if (fstat(fd, &stat_buf) == -1)
		tst_brk(TFAIL | TERRNO, "fstat failed");
	dir_mode = stat_buf.st_mode;

	if ((dir_mode & PERMS) == PERMS)
		tst_res(TPASS, "Functionality of fchmod(%d, "
			"%#o) successful", fd, PERMS);
	else
		tst_res(TFAIL, "%s: Incorrect modes 0%03o, "
			"Expected 0%03o",
			TESTDIR, dir_mode, PERMS);
}

static void setup(void)
{
	SAFE_GETPWNAM(nobody_uid);
	SAFE_MKDIR(TESTDIR, DIR_MODE);
	fd = SAFE_OPEN(TESTDIR, O_RDONLY);
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
