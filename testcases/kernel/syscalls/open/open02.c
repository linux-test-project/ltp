// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Ported to LTP: Wayne Boyer
 *	06/2017 Modified by Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 */
/*
 * DESCRIPTION
 *	1. open a new file without O_CREAT, ENOENT should be returned.
 *	2. open a file with O_RDONLY | O_NOATIME and the caller was not
 *	   privileged, EPERM should be returned.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include "tst_test.h"

#define TEST_FILE	"test_file"
#define TEST_FILE2	"test_file2"

static struct tcase {
	char *filename;
	int flag;
	int exp_errno;
} tcases[] = {
	{TEST_FILE, O_RDWR, ENOENT},
	{TEST_FILE2, O_RDONLY | O_NOATIME, EPERM},
};

void setup(void)
{
	struct passwd *ltpuser;

	SAFE_TOUCH(TEST_FILE2, 0644, NULL);

	ltpuser = SAFE_GETPWNAM("nobody");

	SAFE_SETEUID(ltpuser->pw_uid);
}

static void verify_open(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(open(tc->filename, tc->flag, 0444));

	if (TST_RET != -1) {
		tst_res(TFAIL, "open(%s) succeeded unexpectedly",
			tc->filename);
		return;
	}

	if (tc->exp_errno != TST_ERR) {
		tst_res(TFAIL | TTERRNO,
			"open() should fail with %s",
			tst_strerrno(tc->exp_errno));
		return;
	}

	tst_res(TPASS | TTERRNO, "open() failed as expected");
}

void cleanup(void)
{
	SAFE_SETEUID(0);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.needs_root = 1,
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_open,
};
