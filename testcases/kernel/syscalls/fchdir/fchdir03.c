/*
 * Copyright (C) Bull S.A. 2001
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
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
 *	Testcase for testing that fchdir(2) sets EACCES errno
 *
 * ALGORITHM
 *	1.	create a child process, sets its uid to ltpuser1
 *	2.	this child creates a directory with perm 400,
 *	3.	this child opens the directory and gets a file descriptor
 *	4.	this child attempts to fchdir(2) to the directory created in 2.
 *		and expects to get an EACCES.
 */

#include <errno.h>
#include <pwd.h>
#include "tst_test.h"

#define DIRNAME "fchdir03_dir"

static int fd;

void verify_fchdir(void)
{
	TEST(fchdir(fd));

	if (TEST_RETURN != -1) {
		tst_res(TFAIL, "fchdir() succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO != EACCES) {
		tst_res(TFAIL | TTERRNO, "fchdir() should fail with EACCES");
		return;
	}

	tst_res(TPASS | TTERRNO, "fchdir() failed expectedly");
}

void setup(void)
{
	struct passwd *pw;

	pw = SAFE_GETPWNAM("nobody");
	SAFE_SETEUID(pw->pw_uid);
	SAFE_MKDIR(DIRNAME, 0400);

	fd = SAFE_OPEN(DIRNAME, O_RDONLY);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = verify_fchdir,
	.needs_tmpdir = 1,
	.needs_root = 1,
};
