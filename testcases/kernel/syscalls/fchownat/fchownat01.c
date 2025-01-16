// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2006
 * Copyright (c) Linux Test Project, 2007-2025
 */

/*\
 * [Description]
 *
 * Verify that fchownat() succeeds to change file's ownership if the file
 * descriptor is AT_FDCWD or set by opening a directory.
 */

#define _GNU_SOURCE
#include "tst_test.h"

#define TESTFILE1	"testfile1"
#define TESTFILE2	"testfile2"

static uid_t set_uid = 1000;
static gid_t set_gid = 1000;
static int dir_fd = -1;

static void fchownat_verify(void)
{
	struct stat stat_buf;

	TST_EXP_PASS(fchownat(AT_FDCWD, TESTFILE1, set_uid, set_gid, 0),
		     "fchownat(%d, %s, %d, %d, 0)",
		     AT_FDCWD, TESTFILE1, set_uid, set_gid);

	SAFE_STAT(TESTFILE1, &stat_buf);
	TST_EXP_EQ_LI(stat_buf.st_uid, set_uid);
	TST_EXP_EQ_LI(stat_buf.st_gid, set_gid);

	TST_EXP_PASS(fchownat(dir_fd, TESTFILE2, set_uid, set_gid, 0),
		     "fchownat(%d, %s, %d, %d, 0)",
		     dir_fd, TESTFILE2, set_uid, set_gid);

	SAFE_STAT(TESTFILE2, &stat_buf);
	TST_EXP_EQ_LI(stat_buf.st_uid, set_uid);
	TST_EXP_EQ_LI(stat_buf.st_gid, set_gid);
}

static void setup(void)
{
	dir_fd = SAFE_OPEN("./", O_DIRECTORY);
	SAFE_TOUCH(TESTFILE1, 0600, NULL);
	SAFE_TOUCH(TESTFILE2, 0600, NULL);
}

static void cleanup(void)
{
	if (dir_fd != -1)
		SAFE_CLOSE(dir_fd);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.needs_root = 1,
	.test_all = fchownat_verify,
	.setup = setup,
	.cleanup = cleanup,
};
