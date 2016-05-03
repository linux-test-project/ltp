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

#define FNAME "accessfile"

static struct tcase {
	int mode;
	char *name;
} tcases[] = {
	{F_OK, "F_OK"},
	{X_OK, "X_OK"},
	{W_OK, "W_OK"},
	{R_OK, "R_OK"},
};

static void verify_access(unsigned int n)
{
	struct tcase *tc = tcases + n;

	TEST(access(FNAME, tc->mode));

	if (TEST_RETURN == -1) {
		tst_res(TFAIL | TTERRNO, "access(%s, %s) failed",
		        FNAME, tc->name);
		return;
	}

	tst_res(TPASS, "access(%s, %s)", FNAME, tc->name);
}

static void setup(void)
{
	SAFE_TOUCH(FNAME, 06777, NULL);
}

static struct tst_test test = {
	.tid = "access01",
	.needs_tmpdir = 1,
	.setup = setup,
	.test = verify_access,
	.tcnt = ARRAY_SIZE(tcases),
};
