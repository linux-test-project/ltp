/* Copyright (c) International Business Machines  Corp., 2001
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Verify that user cannot create a directory inside directory owned by another
 * user with restrictive permissions and that the errno is set to EACCESS.
 */

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include "tst_test.h"

#define TESTDIR	 "testdir"
#define TESTSUBDIR "testdir/testdir"

static uid_t nobody_uid, bin_uid;

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
	struct passwd *pw;
	pid_t pid;

	pw = SAFE_GETPWNAM("nobody");
	nobody_uid = pw->pw_uid;
	pw = SAFE_GETPWNAM("bin");
	bin_uid = pw->pw_uid;

	pid = SAFE_FORK();
	if (pid == 0) {
		SAFE_SETREUID(nobody_uid, nobody_uid);
		SAFE_MKDIR(TESTDIR, 0700);
		exit(0);
	}

	tst_reap_children();

	SAFE_SETREUID(bin_uid, bin_uid);
}

static struct tst_test test = {
	.test_all = verify_mkdir,
	.needs_tmpdir = 1,
	.needs_root = 1,
	.setup = setup,
	.forks_child = 1,
};
