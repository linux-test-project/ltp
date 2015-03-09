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
 *    TEST IDENTIFIER	: sched_get_priority_max02
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Test for error conditions
 *
 *    TEST CASE TOTAL	: 1
 *
 *    AUTHOR		: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	Verify that given an invalid scheduling policy,
 *	sched_get_priority_max() returns -1 with errno EINVAL
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 * 	  Execute system call
 *	  Check return code, if system call failed (return=-1) & errno=EINVAL
 *		Test Passed
 *	  Otherwise
 *		Test Failed
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  sched_get_priority_max02 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f]
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
#include "test.h"

#define SCHED_INVALID 1000

static void setup();
static void cleanup();

char *TCID = "sched_get_priority_max02";

int TST_TOTAL = 1;

int main(int ac, char **av)
{

	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * Call sched_get_priority_max(2)
		 */
		TEST(sched_get_priority_max(SCHED_INVALID));

		if ((TEST_RETURN == -1) && (TEST_ERRNO == EINVAL)) {
			tst_resm(TPASS, "Test Passed, Got EINVAL");
		} else {
			tst_resm(TFAIL | TTERRNO,
				 "Test Failed, sched_get_priority_max()"
				 " returned %ld", TEST_RETURN);
		}
	}

	/* cleanup and exit */
	cleanup();
	tst_exit();

}

/* setup() - performs all ONE TIME setup for this test */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

}

/*
 *cleanup() -  performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup(void)
{

}
