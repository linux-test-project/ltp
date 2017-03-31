/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test Description:
 *  Verify that access() succeeds to check the existence or read/write/execute
 *  permissions on a file if the mode argument passed was F_OK/R_OK/W_OK/X_OK.
 *
 *  Also verify that, access() succeeds to test the accessibility of the file
 *  referred to by symbolic link if the pathname points to a symbolic link.
 *
 *  As well as verify that, these test files can be
 *  stat/read/written/executed indeed as root and nobody respectively.
 *
 *	07/2001 Ported by Wayne Boyera
 *	06/2016 Modified by Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 */

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include "tst_test.h"

#define FNAME_F	"file_f"
#define FNAME_R	"file_r"
#define FNAME_W	"file_w"
#define FNAME_X	"file_x"
#define SNAME_F	"symlink_f"
#define SNAME_R	"symlink_r"
#define SNAME_W	"symlink_w"
#define SNAME_X	"symlink_x"

static uid_t uid;

static struct tcase {
	const char *pathname;
	int mode;
	char *name;
	const char *targetname;
} tcases[] = {
	{FNAME_F, F_OK, "F_OK", FNAME_F},
	{FNAME_R, R_OK, "R_OK", FNAME_R},
	{FNAME_W, W_OK, "W_OK", FNAME_W},
	{FNAME_X, X_OK, "X_OK", FNAME_X},
	{SNAME_F, F_OK, "F_OK", FNAME_F},
	{SNAME_R, R_OK, "R_OK", FNAME_R},
	{SNAME_W, W_OK, "W_OK", FNAME_W},
	{SNAME_X, X_OK, "X_OK", FNAME_X}
};

static void access_test(struct tcase *tc, const char *user)
{
	struct stat stat_buf;
	char command[64];

	TEST(access(tc->pathname, tc->mode));

	if (TEST_RETURN == -1) {
		tst_res(TFAIL | TTERRNO, "access(%s, %s) as %s failed",
			tc->pathname, tc->name, user);
		return;
	}

	switch (tc->mode) {
	case F_OK:
		/*
		 * The specified file(or pointed to by symbolic link)
		 * exists, attempt to get its status, if successful,
		 * access() behaviour is correct.
		 */
		TEST(stat(tc->targetname, &stat_buf));

		if (TEST_RETURN == -1) {
			tst_res(TFAIL | TTERRNO, "stat(%s) as %s failed",
				tc->targetname, user);
			return;
		}

		break;
	case R_OK:
		/*
		 * The specified file(or pointed to by symbolic link)
		 * has read access, attempt to open the file with O_RDONLY,
		 * if we get a valid fd, access() behaviour is correct.
		 */
		TEST(open(tc->targetname, O_RDONLY));

		if (TEST_RETURN == -1) {
			tst_res(TFAIL | TTERRNO,
				"open %s with O_RDONLY as %s failed",
				tc->targetname, user);
			return;
		}

		SAFE_CLOSE(TEST_RETURN);

		break;
	case W_OK:
		/*
		 * The specified file(or pointed to by symbolic link)
		 * has write access, attempt to open the file with O_WRONLY,
		 * if we get a valid fd, access() behaviour is correct.
		 */
		TEST(open(tc->targetname, O_WRONLY));

		if (TEST_RETURN == -1) {
			tst_res(TFAIL | TTERRNO,
				"open %s with O_WRONLY as %s failed",
				tc->targetname, user);
			return;
		}

		SAFE_CLOSE(TEST_RETURN);

		break;
	case X_OK:
		/*
		 * The specified file(or pointed to by symbolic link)
		 * has execute access, attempt to execute the executable
		 * file, if successful, access() behaviour is correct.
		 */
		sprintf(command, "./%s", tc->targetname);

		TEST(system(command));

		if (TEST_RETURN != 0) {
			tst_res(TFAIL | TTERRNO, "execute %s as %s failed",
				tc->targetname, user);
			return;
		}

		break;
	default:
		break;
	}

	tst_res(TPASS, "access(%s, %s) as %s behaviour is correct.",
		tc->pathname, tc->name, user);
}

static void verify_access(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	pid_t pid;

	/* test as root */
	access_test(tc, "root");

	/* test as nobody */
	pid = SAFE_FORK();
	if (pid) {
		SAFE_WAITPID(pid, NULL, 0);
	} else {
		SAFE_SETUID(uid);
		access_test(tc, "nobody");
	}
}

static void setup(void)
{
	struct passwd *pw;

	pw = SAFE_GETPWNAM("nobody");

	uid = pw->pw_uid;

	SAFE_TOUCH(FNAME_F, 0000, NULL);
	SAFE_TOUCH(FNAME_R, 0444, NULL);
	SAFE_TOUCH(FNAME_W, 0222, NULL);
	SAFE_TOUCH(FNAME_X, 0555, NULL);
	SAFE_FILE_PRINTF(FNAME_X, "#!/bin/sh\n");

	SAFE_SYMLINK(FNAME_F, SNAME_F);
	SAFE_SYMLINK(FNAME_R, SNAME_R);
	SAFE_SYMLINK(FNAME_W, SNAME_W);
	SAFE_SYMLINK(FNAME_X, SNAME_X);
}

static struct tst_test test = {
	.tid = "access02",
	.tcnt = ARRAY_SIZE(tcases),
	.needs_tmpdir = 1,
	.needs_root = 1,
	.forks_child = 1,
	.setup = setup,
	.test = verify_access,
};
