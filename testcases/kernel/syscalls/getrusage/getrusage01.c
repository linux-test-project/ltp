/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
/**********************************************************
 *
 *    TEST IDENTIFIER	: getrusage01
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Basic test for getrusage(2)
 *
 *    TEST CASE TOTAL	: 2
 *
 *    AUTHOR		: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	This is a Phase I test for the getrusage(2) system call.
 *	It is intended to provide a limited exposure of the system call.
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 * 	  Execute system call
 *	  if return code == 0
 *		Test Passed
 *	  else
 *		Test Failed
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  getrusage01 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f]
 * 			     [-p]
 *			where,  -c n : Run n copies concurrently.
 *				-e   : Turn on errno logging.
 *				-h   : Show help screen
 *				-f   : Turn off functional testing
 *				-i n : Execute test n times.
 *				-I x : Execute test for x seconds.
 *				-p   : Pause for SIGUSR1 before starting
 *				-P x : Pause for x seconds between iterations.
 *				-t   : Turn on syscall timing.
 *
 ****************************************************************/

#include <errno.h>
#include <sched.h>
#include <sys/resource.h>
#include "test.h"

static void setup();
static void cleanup();

char *TCID = "getrusage01";
int who[2] = { RUSAGE_SELF, RUSAGE_CHILDREN };

int TST_TOTAL = 2;

int main(int ac, char **av)
{

	int lc, i;
	struct rusage usage;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			TEST(getrusage(who[i], &usage));

			if (TEST_RETURN == 0)
				tst_resm(TPASS, "getrusage passed");
			else
				tst_resm(TFAIL | TTERRNO, "getrusage failed");
		}
	}

	cleanup();
	tst_exit();

}

void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

}

void cleanup(void)
{
}
