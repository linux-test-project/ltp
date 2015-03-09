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
 *    TEST IDENTIFIER	: sched_get_priority_max01
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Basic test for sched_get_priority_max(2)
 *
 *    TEST CASE TOTAL	: 3
 *
 *    AUTHOR		: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	This is a Phase I test for the sched_get_priority_max(2) system call.
 *	It is intended to provide a limited exposure of the system call.
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 * 	  Execute system call
 *	  Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *	  Otherwise, Issue a PASS message.
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  sched_get_priority_max01 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f]
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

static void setup();
static void cleanup();

char *TCID = "sched_get_priority_max01";

static struct test_case_t {
	char *desc;
	int policy;
	int retval;
} test_cases[] = {
	{
	"Test for SCHED_OTHER", SCHED_OTHER, 0}, {
	"Test for SCHED_FIFO", SCHED_FIFO, 99}, {
	"Test for SCHED_RR", SCHED_RR, 99}
};

int TST_TOTAL = sizeof(test_cases) / sizeof(test_cases[0]);

int main(int ac, char **av)
{

	int lc, ind;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (ind = 0; ind < TST_TOTAL; ind++) {
			/*
			 * Call sched_get_priority_max(2)
			 */
			TEST(sched_get_priority_max(test_cases[ind].policy));

			if (TEST_RETURN == test_cases[ind].retval) {
				tst_resm(TPASS, "%s Passed",
					 test_cases[ind].desc);
			} else {
				tst_resm(TFAIL | TTERRNO, "%s Failed,"
					 "sched_get_priority_max() returned %ld",
					 test_cases[ind].desc, TEST_RETURN);
			}
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
