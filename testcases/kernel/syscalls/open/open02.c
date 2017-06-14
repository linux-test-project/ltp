/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 Ported by Wayne Boyer
 *	06/2017 Modified by Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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

	if (TEST_RETURN != -1) {
		tst_res(TFAIL, "open(%s) succeeded unexpectedly",
			tc->filename);
		return;
	}

	if (tc->exp_errno != TEST_ERRNO) {
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
