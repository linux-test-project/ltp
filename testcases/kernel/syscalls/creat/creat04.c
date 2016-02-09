/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
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
 * along with this program;  if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Testcase to check creat(2) fails with EACCES
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "tst_test.h"

#define DIRNAME "testdir"
#define FILENAME DIRNAME"/file1"

static uid_t nobody_uid;

static struct tcase {
	const char *fname;
} tcases[] = {
	{DIRNAME "/file"},
	{FILENAME}
};

static void child_fn(unsigned int i)
{
	SAFE_SETEUID(nobody_uid);

	TEST(creat(tcases[i].fname, 0444));

	if (TEST_RETURN != -1) {
		SAFE_UNLINK(tcases[i].fname);
		tst_res(TFAIL, "call succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO != EACCES) {
		tst_res(TFAIL | TTERRNO, "Expected EACCES");
		return;
	}

	tst_res(TPASS, "call failed with EACCES as expected");
}

static void verify_creat(unsigned int i)
{
	if (SAFE_FORK() == 0)
		child_fn(i);
}

void setup(void)
{
	struct passwd *pw;
	int fd;

	pw = SAFE_GETPWNAM("nobody");
	nobody_uid = pw->pw_uid;

	SAFE_MKDIR(DIRNAME, 0700);
	fd = SAFE_OPEN(FILENAME, O_RDWR | O_CREAT, 0444);
	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tid = "creat04",
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_creat,
	.needs_tmpdir = 1,
	.needs_root = 1,
	.forks_child = 1,
	.setup = setup,
};
