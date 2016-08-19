/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *   AUTHOR		: William Roske
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
/*
 * Basic test for access(2) using F_OK, R_OK, W_OK and X_OK
 */
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "tst_test.h"

#define FNAME_RWX "accessfile_rwx"
#define FNAME_R   "accessfile_r"
#define FNAME_W   "accessfile_w"
#define FNAME_X   "accessfile_x"

#define DNAME_R   "accessdir_r"
#define DNAME_W   "accessdir_w"
#define DNAME_X   "accessdir_x"
#define DNAME_RW  "accessdir_rw"
#define DNAME_RX  "accessdir_rx"
#define DNAME_WX  "accessdir_wx"

static uid_t uid;

static struct tcase {
	const char *fname;
	int mode;
	char *name;
	int exp_errno;
	/* 1: nobody expected  2: root expected 3: both */
	int exp_user;
} tcases[] = {
	{FNAME_RWX, F_OK, "F_OK", 0, 3},
	{FNAME_RWX, X_OK, "X_OK", 0, 3},
	{FNAME_RWX, W_OK, "W_OK", 0, 3},
	{FNAME_RWX, R_OK, "R_OK", 0, 3},

	{FNAME_RWX, R_OK|W_OK, "R_OK|W_OK", 0, 3},
	{FNAME_RWX, R_OK|X_OK, "R_OK|X_OK", 0, 3},
	{FNAME_RWX, W_OK|X_OK, "W_OK|X_OK", 0, 3},
	{FNAME_RWX, R_OK|W_OK|X_OK, "R_OK|W_OK|X_OK", 0, 3},

	{FNAME_X, X_OK, "X_OK", 0, 3},
	{FNAME_W, W_OK, "W_OK", 0, 3},
	{FNAME_R, R_OK, "R_OK", 0, 3},

	{FNAME_R, X_OK, "X_OK", EACCES, 3},
	{FNAME_R, W_OK, "W_OK", EACCES, 1},
	{FNAME_W, R_OK, "R_OK", EACCES, 1},
	{FNAME_W, X_OK, "X_OK", EACCES, 3},
	{FNAME_X, R_OK, "R_OK", EACCES, 1},
	{FNAME_X, W_OK, "W_OK", EACCES, 1},

	{FNAME_R, W_OK|X_OK, "W_OK|X_OK", EACCES, 3},
	{FNAME_R, R_OK|X_OK, "R_OK|X_OK", EACCES, 3},
	{FNAME_R, R_OK|W_OK, "R_OK|W_OK", EACCES, 1},
	{FNAME_R, R_OK|W_OK|X_OK, "R_OK|W_OK|X_OK", EACCES, 3},

	{FNAME_W, W_OK|X_OK, "W_OK|X_OK", EACCES, 3},
	{FNAME_W, R_OK|X_OK, "R_OK|X_OK", EACCES, 3},
	{FNAME_W, R_OK|W_OK, "R_OK|W_OK", EACCES, 1},
	{FNAME_W, R_OK|W_OK|X_OK, "R_OK|W_OK|X_OK", EACCES, 3},

	{FNAME_X, W_OK|X_OK, "W_OK|X_OK", EACCES, 1},
	{FNAME_X, R_OK|X_OK, "R_OK|X_OK", EACCES, 1},
	{FNAME_X, R_OK|W_OK, "R_OK|W_OK", EACCES, 1},
	{FNAME_X, R_OK|W_OK|X_OK, "R_OK|W_OK|X_OK", EACCES, 1},

	{FNAME_R, W_OK, "W_OK", 0, 2},
	{FNAME_R, R_OK|W_OK, "R_OK|W_OK", 0, 2},

	{FNAME_W, R_OK, "R_OK", 0, 2},
	{FNAME_W, R_OK|W_OK, "R_OK|W_OK", 0, 2},

	{FNAME_X, R_OK, "R_OK", 0, 2},
	{FNAME_X, W_OK, "W_OK", 0, 2},
	{FNAME_X, R_OK|W_OK, "R_OK|W_OK", 0, 2},

	{DNAME_R"/"FNAME_R, F_OK, "F_OK", 0, 2},
	{DNAME_R"/"FNAME_R, R_OK, "R_OK", 0, 2},
	{DNAME_R"/"FNAME_R, W_OK, "W_OK", 0, 2},

	{DNAME_R"/"FNAME_W, F_OK, "F_OK", 0, 2},
	{DNAME_R"/"FNAME_W, R_OK, "R_OK", 0, 2},
	{DNAME_R"/"FNAME_W, W_OK, "W_OK", 0, 2},

	{DNAME_R"/"FNAME_X, F_OK, "F_OK", 0, 2},
	{DNAME_R"/"FNAME_X, R_OK, "R_OK", 0, 2},
	{DNAME_R"/"FNAME_X, W_OK, "W_OK", 0, 2},
	{DNAME_R"/"FNAME_X, X_OK, "X_OK", 0, 2},

	{DNAME_W"/"FNAME_R, F_OK, "F_OK", 0, 2},
	{DNAME_W"/"FNAME_R, R_OK, "R_OK", 0, 2},
	{DNAME_W"/"FNAME_R, W_OK, "W_OK", 0, 2},

	{DNAME_W"/"FNAME_W, F_OK, "F_OK", 0, 2},
	{DNAME_W"/"FNAME_W, R_OK, "R_OK", 0, 2},
	{DNAME_W"/"FNAME_W, W_OK, "W_OK", 0, 2},

	{DNAME_W"/"FNAME_X, F_OK, "F_OK", 0, 2},
	{DNAME_W"/"FNAME_X, R_OK, "R_OK", 0, 2},
	{DNAME_W"/"FNAME_X, W_OK, "W_OK", 0, 2},
	{DNAME_W"/"FNAME_X, X_OK, "X_OK", 0, 2},

	{DNAME_X"/"FNAME_R, F_OK, "F_OK", 0, 3},
	{DNAME_X"/"FNAME_R, R_OK, "R_OK", 0, 3},
	{DNAME_X"/"FNAME_R, W_OK, "W_OK", 0, 2},

	{DNAME_X"/"FNAME_W, F_OK, "F_OK", 0, 3},
	{DNAME_X"/"FNAME_W, R_OK, "R_OK", 0, 2},
	{DNAME_X"/"FNAME_W, W_OK, "W_OK", 0, 3},

	{DNAME_X"/"FNAME_X, F_OK, "F_OK", 0, 3},
	{DNAME_X"/"FNAME_X, R_OK, "R_OK", 0, 2},
	{DNAME_X"/"FNAME_X, W_OK, "W_OK", 0, 2},
	{DNAME_X"/"FNAME_X, X_OK, "X_OK", 0, 3},

	{DNAME_RW"/"FNAME_R, F_OK, "F_OK", 0, 2},
	{DNAME_RW"/"FNAME_R, R_OK, "R_OK", 0, 2},
	{DNAME_RW"/"FNAME_R, W_OK, "W_OK", 0, 2},

	{DNAME_RW"/"FNAME_W, F_OK, "F_OK", 0, 2},
	{DNAME_RW"/"FNAME_W, R_OK, "R_OK", 0, 2},
	{DNAME_RW"/"FNAME_W, W_OK, "W_OK", 0, 2},

	{DNAME_RW"/"FNAME_X, F_OK, "F_OK", 0, 2},
	{DNAME_RW"/"FNAME_X, R_OK, "R_OK", 0, 2},
	{DNAME_RW"/"FNAME_X, W_OK, "W_OK", 0, 2},
	{DNAME_RW"/"FNAME_X, X_OK, "X_OK", 0, 2},

	{DNAME_RX"/"FNAME_R, F_OK, "F_OK", 0, 3},
	{DNAME_RX"/"FNAME_R, R_OK, "R_OK", 0, 3},
	{DNAME_RX"/"FNAME_R, W_OK, "W_OK", 0, 2},

	{DNAME_RX"/"FNAME_W, F_OK, "F_OK", 0, 3},
	{DNAME_RX"/"FNAME_W, R_OK, "R_OK", 0, 2},
	{DNAME_RX"/"FNAME_W, W_OK, "W_OK", 0, 3},

	{DNAME_RX"/"FNAME_X, F_OK, "F_OK", 0, 3},
	{DNAME_RX"/"FNAME_X, R_OK, "R_OK", 0, 2},
	{DNAME_RX"/"FNAME_X, W_OK, "W_OK", 0, 2},
	{DNAME_RX"/"FNAME_X, X_OK, "X_OK", 0, 3},

	{DNAME_WX"/"FNAME_R, F_OK, "F_OK", 0, 3},
	{DNAME_WX"/"FNAME_R, R_OK, "R_OK", 0, 3},
	{DNAME_WX"/"FNAME_R, W_OK, "W_OK", 0, 2},

	{DNAME_WX"/"FNAME_W, F_OK, "F_OK", 0, 3},
	{DNAME_WX"/"FNAME_W, R_OK, "R_OK", 0, 2},
	{DNAME_WX"/"FNAME_W, W_OK, "W_OK", 0, 3},

	{DNAME_WX"/"FNAME_X, F_OK, "F_OK", 0, 3},
	{DNAME_WX"/"FNAME_X, R_OK, "R_OK", 0, 2},
	{DNAME_WX"/"FNAME_X, W_OK, "W_OK", 0, 2},
	{DNAME_WX"/"FNAME_X, X_OK, "X_OK", 0, 3},

	{DNAME_R"/"FNAME_R, F_OK, "F_OK", EACCES, 1},
	{DNAME_R"/"FNAME_R, R_OK, "R_OK", EACCES, 1},
	{DNAME_R"/"FNAME_R, W_OK, "W_OK", EACCES, 1},
	{DNAME_R"/"FNAME_R, X_OK, "X_OK", EACCES, 3},

	{DNAME_R"/"FNAME_W, F_OK, "F_OK", EACCES, 1},
	{DNAME_R"/"FNAME_W, R_OK, "R_OK", EACCES, 1},
	{DNAME_R"/"FNAME_W, W_OK, "W_OK", EACCES, 1},
	{DNAME_R"/"FNAME_W, X_OK, "X_OK", EACCES, 3},

	{DNAME_R"/"FNAME_X, F_OK, "F_OK", EACCES, 1},
	{DNAME_R"/"FNAME_X, R_OK, "R_OK", EACCES, 1},
	{DNAME_R"/"FNAME_X, W_OK, "W_OK", EACCES, 1},
	{DNAME_R"/"FNAME_X, X_OK, "X_OK", EACCES, 1},

	{DNAME_W"/"FNAME_R, F_OK, "F_OK", EACCES, 1},
	{DNAME_W"/"FNAME_R, R_OK, "R_OK", EACCES, 1},
	{DNAME_W"/"FNAME_R, W_OK, "W_OK", EACCES, 1},
	{DNAME_W"/"FNAME_R, X_OK, "X_OK", EACCES, 3},

	{DNAME_W"/"FNAME_W, F_OK, "F_OK", EACCES, 1},
	{DNAME_W"/"FNAME_W, R_OK, "R_OK", EACCES, 1},
	{DNAME_W"/"FNAME_W, W_OK, "W_OK", EACCES, 1},
	{DNAME_W"/"FNAME_W, X_OK, "X_OK", EACCES, 3},

	{DNAME_W"/"FNAME_X, F_OK, "F_OK", EACCES, 1},
	{DNAME_W"/"FNAME_X, R_OK, "R_OK", EACCES, 1},
	{DNAME_W"/"FNAME_X, W_OK, "W_OK", EACCES, 1},
	{DNAME_W"/"FNAME_X, X_OK, "X_OK", EACCES, 1},

	{DNAME_X"/"FNAME_R, W_OK, "W_OK", EACCES, 1},
	{DNAME_X"/"FNAME_R, X_OK, "X_OK", EACCES, 3},

	{DNAME_X"/"FNAME_W, R_OK, "R_OK", EACCES, 1},
	{DNAME_X"/"FNAME_W, X_OK, "X_OK", EACCES, 3},

	{DNAME_X"/"FNAME_X, R_OK, "R_OK", EACCES, 1},
	{DNAME_X"/"FNAME_X, W_OK, "W_OK", EACCES, 1},

	{DNAME_RW"/"FNAME_R, F_OK, "F_OK", EACCES, 1},
	{DNAME_RW"/"FNAME_R, R_OK, "R_OK", EACCES, 1},
	{DNAME_RW"/"FNAME_R, W_OK, "W_OK", EACCES, 1},
	{DNAME_RW"/"FNAME_R, X_OK, "X_OK", EACCES, 3},

	{DNAME_RW"/"FNAME_W, F_OK, "F_OK", EACCES, 1},
	{DNAME_RW"/"FNAME_W, R_OK, "R_OK", EACCES, 1},
	{DNAME_RW"/"FNAME_W, W_OK, "W_OK", EACCES, 1},
	{DNAME_RW"/"FNAME_W, X_OK, "X_OK", EACCES, 3},

	{DNAME_RW"/"FNAME_X, F_OK, "F_OK", EACCES, 1},
	{DNAME_RW"/"FNAME_X, R_OK, "R_OK", EACCES, 1},
	{DNAME_RW"/"FNAME_X, W_OK, "W_OK", EACCES, 1},
	{DNAME_RW"/"FNAME_X, X_OK, "X_OK", EACCES, 1},

	{DNAME_RX"/"FNAME_R, W_OK, "W_OK", EACCES, 1},
	{DNAME_RX"/"FNAME_R, X_OK, "X_OK", EACCES, 3},

	{DNAME_RX"/"FNAME_W, R_OK, "R_OK", EACCES, 1},
	{DNAME_RX"/"FNAME_W, X_OK, "X_OK", EACCES, 3},

	{DNAME_RX"/"FNAME_X, R_OK, "R_OK", EACCES, 1},
	{DNAME_RX"/"FNAME_X, W_OK, "W_OK", EACCES, 1},

	{DNAME_WX"/"FNAME_R, W_OK, "W_OK", EACCES, 1},
	{DNAME_WX"/"FNAME_R, X_OK, "X_OK", EACCES, 3},

	{DNAME_WX"/"FNAME_W, R_OK, "R_OK", EACCES, 1},
	{DNAME_WX"/"FNAME_W, X_OK, "X_OK", EACCES, 3},

	{DNAME_WX"/"FNAME_X, R_OK, "R_OK", EACCES, 1},
	{DNAME_WX"/"FNAME_X, W_OK, "W_OK", EACCES, 1}
};

static void verify_success(struct tcase *tc, const char *user)
{
	if (TEST_RETURN == -1) {
		tst_res(TFAIL | TTERRNO,
		        "access(%s, %s) as %s failed unexpectedly",
		        tc->fname, tc->name, user);
		return;
	}

	tst_res(TPASS, "access(%s, %s) as %s", tc->fname, tc->name, user);
}

static void verify_failure(struct tcase *tc, const char *user)
{
	if (TEST_RETURN != -1) {
		tst_res(TFAIL, "access(%s, %s) as %s succeded unexpectedly",
		        tc->fname, tc->name, user);
		return;
	}

	if (TEST_ERRNO != tc->exp_errno) {
		tst_res(TFAIL | TTERRNO,
		        "access(%s, %s) as %s should fail with %s",
		        tc->fname, tc->name, user,
		        tst_strerrno(tc->exp_errno));
		return;
	}

	tst_res(TPASS | TTERRNO, "access(%s, %s) as %s",
	        tc->fname, tc->name, user);
}

static void access_test(struct tcase *tc, const char *user)
{
	TEST(access(tc->fname, tc->mode));

	if (tc->exp_errno)
		verify_failure(tc, user);
	else
		verify_success(tc, user);
}

static void verify_access(unsigned int n)
{
	struct tcase *tc = tcases + n;
	pid_t pid;

	if (tc->exp_user & 0x02)
		access_test(tc, "root");

	if (tc->exp_user & 0x01) {
		pid = SAFE_FORK();
		if (pid) {
			SAFE_WAITPID(pid, NULL, 0);
		} else {
			SAFE_SETUID(uid);
			access_test(tc, "nobody");
		}
	}
}

static void setup(void)
{
	struct passwd *pw;

	pw = SAFE_GETPWNAM("nobody");

	uid = pw->pw_uid;

	SAFE_TOUCH(FNAME_RWX, 0777, NULL);
	SAFE_TOUCH(FNAME_R, 0444, NULL);
	SAFE_TOUCH(FNAME_W, 0222, NULL);
	SAFE_TOUCH(FNAME_X, 0111, NULL);

	SAFE_MKDIR(DNAME_R, 0444);
	SAFE_MKDIR(DNAME_W, 0222);
	SAFE_MKDIR(DNAME_X, 0111);
	SAFE_MKDIR(DNAME_RW, 0666);
	SAFE_MKDIR(DNAME_RX, 0555);
	SAFE_MKDIR(DNAME_WX, 0333);

	SAFE_TOUCH(DNAME_R"/"FNAME_R, 0444, NULL);
	SAFE_TOUCH(DNAME_R"/"FNAME_W, 0222, NULL);
	SAFE_TOUCH(DNAME_R"/"FNAME_X, 0111, NULL);

	SAFE_TOUCH(DNAME_W"/"FNAME_R, 0444, NULL);
	SAFE_TOUCH(DNAME_W"/"FNAME_W, 0222, NULL);
	SAFE_TOUCH(DNAME_W"/"FNAME_X, 0111, NULL);

	SAFE_TOUCH(DNAME_X"/"FNAME_R, 0444, NULL);
	SAFE_TOUCH(DNAME_X"/"FNAME_W, 0222, NULL);
	SAFE_TOUCH(DNAME_X"/"FNAME_X, 0111, NULL);

	SAFE_TOUCH(DNAME_RW"/"FNAME_R, 0444, NULL);
	SAFE_TOUCH(DNAME_RW"/"FNAME_W, 0222, NULL);
	SAFE_TOUCH(DNAME_RW"/"FNAME_X, 0111, NULL);

	SAFE_TOUCH(DNAME_RX"/"FNAME_R, 0444, NULL);
	SAFE_TOUCH(DNAME_RX"/"FNAME_W, 0222, NULL);
	SAFE_TOUCH(DNAME_RX"/"FNAME_X, 0111, NULL);

	SAFE_TOUCH(DNAME_WX"/"FNAME_R, 0444, NULL);
	SAFE_TOUCH(DNAME_WX"/"FNAME_W, 0222, NULL);
	SAFE_TOUCH(DNAME_WX"/"FNAME_X, 0111, NULL);
}

static struct tst_test test = {
	.tid = "access01",
	.needs_tmpdir = 1,
	.needs_root = 1,
	.forks_child = 1,
	.setup = setup,
	.test = verify_access,
	.tcnt = ARRAY_SIZE(tcases),
};
