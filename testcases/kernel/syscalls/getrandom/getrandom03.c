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
#include "test.h"

#define MAX_SIZE 256

char *TCID = "getrandom03";
int TST_TOTAL = 5;

static char buf[256];

int main(int ac, char **av)
{
	int lc, i;
	long size;

	tst_parse_opts(ac, av, NULL, NULL);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++) {
			size = random() % MAX_SIZE;
			TEST(ltp_syscall(__NR_getrandom, buf, size, 0));
			if (TEST_RETURN != size)
				tst_resm(TFAIL | TTERRNO, "getrandom failed");
			else
				tst_resm(TPASS, "getrandom returned %ld",
					TEST_RETURN);
		}
	}
	tst_exit();
}
