// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 */

/*
 * DESCRIPTION
 *   check rmdir() fails with EPERM or EACCES
 *
 *	1. Create a directory tstdir1 and set the sticky bit, then
 *         create directory tstdir2 under tstdir1. Call rmdir(),
 *         set to be user nobody. Pass tstdir2 to rmdir(2), verify
 *         the return value is not 0 and the errno is EPERM or EACCES.
 *
 *	2. Create a directory tstdir1 and doesn't give execute/search
 *         permission to nobody, then create directory tstdir2 under
 *         tstdir1. Call rmdir(), set to be user nobody. Pass
 *         tstdir2 to rmdir(2), verify the return value is not 0 and
 *         the errno is EACCES.
 */
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include "tst_test.h"

#define DIR_MODE    0777
#define NOEXCUTE_MODE 0766
#define TESTDIR     "testdir"
#define TESTDIR2    "testdir/testdir2"
#define TESTDIR3    "testdir3"
#define TESTDIR4    "testdir3/testdir4"

static struct testcase {
	mode_t dir_mode;
	char *subdir;
} tcases[] =  {
	{DIR_MODE | S_ISVTX, TESTDIR2},
	{NOEXCUTE_MODE, TESTDIR4},
};

static void do_rmdir(unsigned int n)
{
	struct testcase *tc = &tcases[n];

	TEST(rmdir(tc->subdir));
	if (TST_RET != -1) {
		tst_res(TFAIL, "rmdir() succeeded unexpectedly");
		return;
	}

	if (TST_ERR != EACCES) {
		if (tc->dir_mode & S_ISVTX && TST_ERR == EPERM)
			tst_res(TPASS | TTERRNO, "rmdir() got expected errno");
		else
			tst_res(TFAIL | TTERRNO, "expected EPERM, but got");
		return;
	}

	tst_res(TPASS | TTERRNO, "rmdir() got expected errno");
}


static void setup(void)
{
	struct passwd *pw;
	pw = SAFE_GETPWNAM("nobody");

	umask(0);

	SAFE_MKDIR(TESTDIR, DIR_MODE | S_ISVTX);
	SAFE_MKDIR(TESTDIR2, DIR_MODE);
	SAFE_MKDIR(TESTDIR3, NOEXCUTE_MODE);
	SAFE_MKDIR(TESTDIR4, DIR_MODE);

	SAFE_SETEUID(pw->pw_uid);
}

static void cleanup(void)
{
	SAFE_SETEUID(0);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = do_rmdir,
	.needs_root = 1,
	.needs_tmpdir = 1,
};

