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

#include <sys/syscall.h>
#include <errno.h>
#include <linux/random.h>
#include <sys/resource.h>
#include "test.h"


char *TCID = "getrandom04";
int TST_TOTAL = 1;
static char buf[128];

int main(int ac, char **av)
{
	tst_parse_opts(ac, av, NULL, NULL);

	int r;

	struct rlimit lim = {3, 3};

	r = setrlimit(RLIMIT_NOFILE, &lim);

	if (r == -1)
		tst_brkm(TBROK | TTERRNO, NULL, "setrlimit failed");

	TEST(syscall(SYS_getrandom, buf, 100, 0));
	if (TEST_RETURN == -1 && TEST_ERRNO == ENOSYS)
		tst_brkm(TCONF, NULL,
			"This test needs kernel 3.17 or newer");

	if (TEST_RETURN == -1)
		tst_resm(TFAIL | TTERRNO, "getrandom failed");
	else
		tst_resm(TPASS, "getrandom returned %ld", TEST_RETURN);

	tst_exit();
}

