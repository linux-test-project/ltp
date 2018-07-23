/*
 * Copyright (c) International Business Machines  Corp., 2001
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

/* DESCRIPTION
 *   This test will verify that rmdir(2) syscall basic functionality.
 *   verify rmdir(2) returns a value of 0 and the directory being removed.
 */
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "tst_test.h"

#define TESTDIR "testdir"

static void verify_rmdir(void)
{
	struct stat buf;

	SAFE_MKDIR(TESTDIR, 0777);

	TEST(rmdir(TESTDIR));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "rmdir(%s) failed", TESTDIR);
		return;
	}

	if (!stat(TESTDIR, &buf))
		tst_res(TFAIL, "rmdir(%s) failed", TESTDIR);
	else
		tst_res(TPASS, "rmdir(%s) success", TESTDIR);
}

static struct tst_test test = {
	.test_all = verify_rmdir,
	.needs_tmpdir = 1,
};

