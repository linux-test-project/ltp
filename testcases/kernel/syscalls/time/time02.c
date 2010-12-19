/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * Test Name: time02
 *
 * Test Description:
 *  Verify that time(2) returns the value of time in seconds since
 *  the Epoch and stores this value in the memory pointed to by the parameter.
 *
 * Expected Result:
 *  time() should return the time (seconds) since the Epoch and this value
 *  should be equal to the value stored in the specified parameter.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Create temporary directory.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed (return=-1)
 *	Log the errno and Issue a FAIL message.
 *   Otherwise,
 *	Verify the Functionality of system call
 *      if successful,
 *		Issue Functionality-Pass message.
 *      Otherwise,
 *		Issue Functionality-Fail message.
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *
 * Usage:  <for command-line>
 *  time02 [-c n] [-e] [-f] [-i n] [-I x] [-p x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-f   : Turn off functionality Testing.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions:
 *  None.
 *
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <stdint.h>

#include "test.h"
#include "usctest.h"

void setup();			/* setup function for the test */
void cleanup();			/* cleanup function for the test */

char *TCID = "time02";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
int exp_enos[] = { 0 };

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	time_t tloc;		/* time_t variables for time(2) */

	/* Parse standard options given to run the test. */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/*
		 * Call time() to get the time in seconds$
		 * since Epoch.
		 */
		TEST(time(&tloc));

		/* Check return code from time(2) */
		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL, "time(0) Failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			/*
			 * Perform functional verification if test executed
			 * without (-f) option.
			 */
			if (STD_FUNCTIONAL_TEST) {
				if (tloc == TEST_RETURN) {
					tst_resm(TPASS, "time() returned value "
						 "%ld, stored value %jd are same",
						 TEST_RETURN, (intmax_t)tloc);
				} else {
					tst_resm(TFAIL, "time() returned value "
						 "%ld, stored value %jd are "
						 "different", TEST_RETURN,
						 (intmax_t)tloc);
				}
			} else {
				tst_resm(TPASS, "call succeeded");
			}

		}
		Tst_count++;	/* incr. TEST_LOOP counter */
	}

	/* cleanup and exit */
	cleanup();
	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}