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
#include "tst_test.h"

#define FNAME_RWX "accessfile_rwx"
#define FNAME_R   "accesfile_r"
#define FNAME_W   "accesfile_w"
#define FNAME_X   "accesfile_x"

static struct tcase {
	const char *fname;
	int mode;
	char *name;
	int exp_errno;
} tcases[] = {
	{FNAME_RWX, F_OK, "F_OK", 0},
	{FNAME_RWX, X_OK, "X_OK", 0},
	{FNAME_RWX, W_OK, "W_OK", 0},
	{FNAME_RWX, R_OK, "R_OK", 0},

	{FNAME_RWX, R_OK|W_OK, "R_OK|W_OK", 0},
	{FNAME_RWX, R_OK|X_OK, "R_OK|X_OK", 0},
	{FNAME_RWX, W_OK|X_OK, "W_OK|X_OK", 0},
	{FNAME_RWX, R_OK|W_OK|X_OK, "R_OK|W_OK|X_OK", 0},

	{FNAME_X, X_OK, "X_OK", 0},
	{FNAME_W, W_OK, "W_OK", 0},
	{FNAME_R, R_OK, "R_OK", 0},

	{FNAME_R, X_OK, "X_OK", EACCES},
	{FNAME_R, W_OK, "W_OK", EACCES},
	{FNAME_W, R_OK, "R_OK", EACCES},
	{FNAME_W, X_OK, "X_OK", EACCES},
	{FNAME_X, R_OK, "R_OK", EACCES},
	{FNAME_X, W_OK, "W_OK", EACCES},

	{FNAME_R, W_OK|X_OK, "W_OK|X_OK", EACCES},
	{FNAME_R, R_OK|X_OK, "R_OK|X_OK", EACCES},
	{FNAME_R, R_OK|W_OK, "R_OK|W_OK", EACCES},
	{FNAME_R, R_OK|W_OK|X_OK, "R_OK|W_OK|X_OK", EACCES},

	{FNAME_W, W_OK|X_OK, "W_OK|X_OK", EACCES},
	{FNAME_W, R_OK|X_OK, "R_OK|X_OK", EACCES},
	{FNAME_W, R_OK|W_OK, "R_OK|W_OK", EACCES},
	{FNAME_W, R_OK|W_OK|X_OK, "R_OK|W_OK|X_OK", EACCES},

	{FNAME_X, W_OK|X_OK, "W_OK|X_OK", EACCES},
	{FNAME_X, R_OK|X_OK, "R_OK|X_OK", EACCES},
	{FNAME_X, R_OK|W_OK, "R_OK|W_OK", EACCES},
	{FNAME_X, R_OK|W_OK|X_OK, "R_OK|W_OK|X_OK", EACCES},
};

static void verify_success(struct tcase *tc)
{
	if (TEST_RETURN == -1) {
		tst_res(TFAIL | TTERRNO, "access(%s, %s) failed unexpectedly",
		        tc->fname, tc->name);
		return;
	}

	tst_res(TPASS, "access(%s, %s)", tc->fname, tc->name);
}

static void verify_failure(struct tcase *tc)
{
	if (TEST_RETURN != -1) {
		tst_res(TFAIL, "access(%s, %s) succeded unexpectedly",
		        tc->fname, tc->name);
		return;
	}

	if (TEST_ERRNO != tc->exp_errno) {
		tst_res(TFAIL | TTERRNO, "access(%s, %s) should fail with %s",
		        tc->fname, tc->name, tst_strerrno(tc->exp_errno));
		return;
	}

	tst_res(TPASS | TTERRNO, "access(%s, %s)", tc->fname, tc->name);
}

static void verify_access(unsigned int n)
{
	struct tcase *tc = tcases + n;

	TEST(access(tc->fname, tc->mode));

	if (tc->exp_errno)
		verify_failure(tc);
	else
		verify_success(tc);
}

static void setup(void)
{
	SAFE_TOUCH(FNAME_RWX, 0777, NULL);
	SAFE_TOUCH(FNAME_R, 0444, NULL);
	SAFE_TOUCH(FNAME_W, 0222, NULL);
	SAFE_TOUCH(FNAME_X, 0111, NULL);
}

static struct tst_test test = {
	.tid = "access01",
	.needs_tmpdir = 1,
	.setup = setup,
	.test = verify_access,
	.tcnt = ARRAY_SIZE(tcases),
};
