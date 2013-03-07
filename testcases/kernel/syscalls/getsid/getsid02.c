/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *   Copyright (c) 2012 Cyril Hrubis <chrubis@suse.cz>
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

#define _GNU_SOURCE 1

#include "test.h"
#include "usctest.h"

#include <errno.h>

char *TCID = "getsid02";
int TST_TOTAL = 1;

static unsigned long pid_max;

static void cleanup(void);
static void setup(void);

int exp_enos[] = { ESRCH, 0 };

int main(int ac, char **av)
{
	int lc;
	char *msg;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		TEST(getsid(pid_max + 1));

		if (TEST_RETURN == 0) {
			tst_resm(TFAIL, "call succeed when failure expected");
			continue;
		}

		switch (TEST_ERRNO) {
		case ESRCH:
			tst_resm(TPASS, "expected failure - errno = %d - %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			break;
		default:
			tst_resm(TFAIL, "call failed to produce "
				 "expected error - errno = %d - %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			break;
		}
	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	SAFE_FILE_SCANF(NULL, "/proc/sys/kernel/pid_max", "%lu", &pid_max);

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;
}

void cleanup(void)
{
	TEST_CLEANUP;
}
