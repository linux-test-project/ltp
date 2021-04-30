// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 */

/*\
 * [Description]
 *
 * Verify that, chown(2) succeeds to change the owner and group of a file
 * specified by path to any numeric owner(uid)/group(gid) values when invoked
 * by super-user.
 */

#include "tst_test.h"
#include "compat_tst_16.h"
#include "tst_safe_macros.h"

#define FILE_MODE (S_IFREG|S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define TESTFILE "testfile"

struct test_case_t {
	uid_t uid;
	gid_t gid;
} tc[] = {
	{700, 701},
	{702, 701},
	{702, 703},
	{704, 705}
};

static void run(unsigned int i)
{
	struct stat stat_buf;
	TST_EXP_PASS(CHOWN(TESTFILE, tc[i].uid, tc[i].gid));

	SAFE_STAT(TESTFILE, &stat_buf);
	if (stat_buf.st_uid != tc[i].uid || stat_buf.st_gid != tc[i].gid) {
		tst_res(TFAIL, "%s: incorrect ownership set, expected %d %d",
			TESTFILE, tc[i].uid, tc[i].gid);
	}
}

static void setup(void)
{
	SAFE_TOUCH(TESTFILE, FILE_MODE, NULL);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tc),
	.needs_root = 1,
	.needs_tmpdir = 1,
	.setup = setup,
	.test = run,
};
