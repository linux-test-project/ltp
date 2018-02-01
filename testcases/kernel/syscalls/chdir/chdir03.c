/*
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
 * along with this program;  if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * DESCRIPTION
 *	Testcase for testing that chdir(2) sets EACCES errno
 *
 * ALGORITHM
 *	1.	running as root, create a directory with perm 700
 *	2.	create a child process, sets its uid to nobody
 *	3.	this child attempts to chdir(2) to the directory created in 1
 *		and expects to get an EACCES.
 */

#include <pwd.h>
#include <errno.h>
#include <stdlib.h>
#include "tst_test.h"

#define DIRNAME "chdir03_dir"

static uid_t nobody_uid;

void verify_chdir(void)
{
	pid_t pid;

	pid = SAFE_FORK();
	if (!pid) {
		SAFE_SETUID(nobody_uid);

		TEST(chdir(DIRNAME));

		if (TEST_RETURN != -1) {
			tst_res(TFAIL, "chdir() succeeded unexpectedly");
			return;
		}

		if (TEST_ERRNO != EACCES) {
			tst_res(TFAIL | TTERRNO,
				"chdir() should fail with EACCES");
			return;
		}

		tst_res(TPASS | TTERRNO, "chdir() failed expectedly");
	}
}

void setup(void)
{
	struct passwd *pw;

	pw = SAFE_GETPWNAM("nobody");
	nobody_uid = pw->pw_uid;

	SAFE_MKDIR(DIRNAME, 0700);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = verify_chdir,
	.needs_tmpdir = 1,
	.needs_root = 1,
	.forks_child = 1,
};
