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
 */

/*
 * AUTHOR   : Cédric Hnyda
 * DATE STARTED : 06/13/2015
 *
 *  Calls getrandom(2) with a NULL buffer and expects failure.
 *
 */

#include <sys/syscall.h>
#include <errno.h>
#include <linux/random.h>
#include "test.h"

static int modes[] = {0, GRND_RANDOM, GRND_NONBLOCK,
						GRND_RANDOM | GRND_NONBLOCK};

char *TCID = "getrandom01";
int TST_TOTAL = ARRAY_SIZE(modes);

int main(int ac, char **av)
{
	int lc, i;

	tst_parse_opts(ac, av, NULL, NULL);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			TEST(syscall(SYS_getrandom, NULL, 100, modes[i]));

			if (TEST_RETURN == -1 && TEST_ERRNO == ENOSYS)
				tst_brkm(TCONF, NULL,
						"This test needs kernel 3.17 or newer");

			if (TEST_RETURN == -1)
				tst_resm(TPASS, "getrandom returned %ld",
						TEST_RETURN);
			else
				tst_resm(TFAIL | TTERRNO, "getrandom failed");
		}
	}
	tst_exit();
}
