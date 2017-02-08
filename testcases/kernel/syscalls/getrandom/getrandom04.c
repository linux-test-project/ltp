/*
 * Copyright (C) 2015 Cedric Hnyda ced.hnyda@gmail.com
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
 * AUTHOR   : Cédric Hnyda
 * DATE STARTED : 06/13/2015
 *
 *  Calls getrandom(2) after having limited the number of available file
 *  descriptors to 3 and expects success.
 */

#include <sys/resource.h>
#include "lapi/getrandom.h"
#include "linux_syscall_numbers.h"
#include "tst_test.h"

static void verify_getrandom(void)
{
	char buf[128];
	struct rlimit lold, lnew;

	SAFE_GETRLIMIT(RLIMIT_NOFILE, &lold);
	lnew.rlim_max = lold.rlim_max;
	lnew.rlim_cur = 3;
	SAFE_SETRLIMIT(RLIMIT_NOFILE, &lnew);

	TEST(tst_syscall(__NR_getrandom, buf, 100, 0));
	if (TEST_RETURN == -1)
		tst_res(TFAIL | TTERRNO, "getrandom failed");
	else
		tst_res(TPASS, "getrandom returned %ld", TEST_RETURN);

	SAFE_SETRLIMIT(RLIMIT_NOFILE, &lold);
}

static struct tst_test test = {
	.tid = "getrandom04",
	.test_all = verify_getrandom,
};
