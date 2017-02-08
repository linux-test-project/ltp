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
 * AUTHOR	: Cédric Hnyda
 * DATE STARTED	: 06/13/2015
 *
 *  Calls getrandom(2), check that the return value is equal to the
 *  number of bytes required and expects success.
 */

#include "lapi/getrandom.h"
#include "linux_syscall_numbers.h"
#include "tst_test.h"

#define MAX_SIZE 256

static unsigned int sizes[] = {
	1,
	2,
	3,
	7,
	8,
	15,
	22,
	64,
	127,
};

static void verify_getrandom(unsigned int n)
{
	char buf[MAX_SIZE];

	TEST(tst_syscall(__NR_getrandom, buf, sizes[n], 0));

	if (TEST_RETURN != sizes[n]) {
		tst_res(TFAIL | TTERRNO, "getrandom returned %li, expected %u",
			TEST_RETURN, sizes[n]);
	} else {
		tst_res(TPASS, "getrandom returned %ld", TEST_RETURN);
	}
}

static struct tst_test test = {
	.tid = "getrandom03",
	.tcnt = ARRAY_SIZE(sizes),
	.test = verify_getrandom,
};
