// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 */

/*  DESCRIPTION
 *  This test will verify that new directory created by mkdir(2) inherites
 *  the group ID from the parent directory and S_ISGID bit, if the S_ISGID
 *  bit is set in the parent directory.
 */

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include "tst_test.h"

#define TESTDIR1	"testdir1"
#define TESTDIR2	"testdir1/testdir2"

static uid_t nobody_uid, bin_uid;
static gid_t nobody_gid, bin_gid;

static void verify_mkdir(void)
{
	struct stat buf1, buf2;
	pid_t pid;
	int fail = 0;

	pid = SAFE_FORK();
	if (pid == 0) {
		SAFE_SETREGID(bin_gid, bin_gid);
		SAFE_SETREUID(bin_uid, bin_uid);
		SAFE_MKDIR(TESTDIR2, 0777);

		SAFE_STAT(TESTDIR2, &buf2);
		SAFE_STAT(TESTDIR1, &buf1);

		if (buf2.st_gid != buf1.st_gid) {
			tst_res(TFAIL,
				"New dir FAILED to inherit GID have %d expected %d",
				buf2.st_gid, buf1.st_gid);
			fail = 1;
		}

		if (!(buf2.st_mode & S_ISGID)) {
			tst_res(TFAIL, "New dir FAILED to inherit S_ISGID");
			fail = 1;
		}

		if (!fail)
			tst_res(TPASS, "New dir inherited GID and S_ISGID");

		exit(0);
	}

	tst_reap_children();
	SAFE_RMDIR(TESTDIR2);
}


static void setup(void)
{
	struct passwd *pw;
	struct stat buf;
	pid_t pid;

	pw = SAFE_GETPWNAM("nobody");
	nobody_uid = pw->pw_uid;
	nobody_gid = pw->pw_gid;
	pw = SAFE_GETPWNAM("bin");
	bin_uid = pw->pw_uid;
	bin_gid = pw->pw_gid;

	umask(0);

	pid = SAFE_FORK();
	if (pid == 0) {
		SAFE_SETREGID(nobody_gid, nobody_gid);
		SAFE_SETREUID(nobody_uid, nobody_uid);
		SAFE_MKDIR(TESTDIR1, 0777);
		SAFE_STAT(TESTDIR1, &buf);
		SAFE_CHMOD(TESTDIR1, buf.st_mode | S_ISGID);
		exit(0);
	}

	tst_reap_children();
}

static struct tst_test test = {
	.setup = setup,
	.needs_tmpdir = 1,
	.needs_root = 1,
	.test_all = verify_mkdir,
	.forks_child = 1,
};
