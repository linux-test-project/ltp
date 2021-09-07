// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) International Business Machines Corp., 2001
 */

/*
 * Verify that user cannot create a directory inside directory owned by another
 * user with restrictive permissions and that the errno is set to EACCESS.
 */

#include <errno.h>
#include <sys/types.h>
#include <pwd.h>
#include "tst_test.h"
#include "tst_uid.h"

#define TESTDIR	 "testdir"
#define TESTSUBDIR "testdir/testdir"

static void verify_mkdir(void)
{
	if (mkdir(TESTSUBDIR, 0777) != -1) {
		tst_res(TFAIL, "mkdir(%s, %#o) succeeded unexpectedly",
			TESTSUBDIR, 0777);
		return;
	}

	if (errno != EACCES) {
		tst_res(TFAIL | TERRNO, "Expected EACCES got");
		return;
	}

	tst_res(TPASS | TERRNO, "mkdir() failed expectedly");
}

static void setup(void)
{
	uid_t test_users[2];

	tst_get_uids(test_users, 0, 2);

	SAFE_MKDIR(TESTDIR, 0700);
	SAFE_CHOWN(TESTDIR, test_users[0], getgid());

	SAFE_SETREUID(test_users[1], test_users[1]);
}

static struct tst_test test = {
	.test_all = verify_mkdir,
	.needs_tmpdir = 1,
	.needs_root = 1,
	.setup = setup,
};
