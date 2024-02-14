// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *  Authors:	William Roske, Dave Fenner
 *
 *  06/2019 Ported to new library:
 *		Christian Amann <camann@suse.com>
 */
/*
 * Basic test for lstat():
 *
 * Tests if lstat() writes correct information about a symlink
 * into the stat structure.
 */

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "tst_test.h"

#define TESTFILE        "tst_file"
#define TESTSYML        "tst_syml"

static uid_t user_id;
static gid_t group_id;

static void run(void)
{
	struct stat stat_buf;

	memset(&stat_buf, 0, sizeof(stat_buf));

	TEST(lstat(TESTSYML, &stat_buf));
	if (TST_RET == -1)
		tst_res(TFAIL | TTERRNO, "Calling lstat() failed");

	if ((stat_buf.st_mode & S_IFMT) != S_IFLNK ||
	    stat_buf.st_uid != user_id ||
	    stat_buf.st_gid != group_id ||
	    stat_buf.st_size != strlen(TESTFILE)) {
		tst_res(TFAIL,
			"lstat() reported incorrect values for the symlink!");
	} else {
		tst_res(TPASS,
			"lstat() reported correct values for the symlink!");
	}
}

static void setup(void)
{
	user_id  = getuid();
	group_id = getgid();

	SAFE_TOUCH(TESTFILE, 0644, NULL);
	SAFE_SYMLINK(TESTFILE, TESTSYML);
}

static void cleanup(void)
{
	SAFE_UNLINK(TESTSYML);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
