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
 * Test Name: nice03
 *
 * Test Description:
 *  Verify that any user can successfully increase the nice value of
 *  the process by passing an increment value (< max. applicable limits) to
 *  nice() system call.
 *
 * Expected Result:
 *  nice() should return value 0 on success and the user should succeed
 *  to increase the nice value of test process.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed (return=-1)
 *   	Log the errno and Issue a FAIL message.
 *   Otherwise,
 *   	Verify the Functionality of system call
 *      if successful,
 *      	Issue Functionality-Pass message.
 *      Otherwise,
 *		Issue Functionality-Fail message.
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *
 * Usage:  <for command-line>
 *  nice03 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS:
 *  The maximum iterations with the -i option is 9
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "test.h"
#include "usctest.h"

char *TCID = "nice03";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

#define	NICEINC		2
int Org_nice;			/* original priority of the test process */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int New_nice;		/* priority of process after nice() */

	/* Parse standard options given to run the test. */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/*
		 * Call nice(2) with an 'incr' parameter set
		 * to a +ve value < max. applicable limit.
		 * (Linux - 20)
		 */
		TEST(nice(NICEINC));

		/* check return code */
		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "nice(%d) Failed, errno=%d : %s",
				 NICEINC, TEST_ERRNO, strerror(TEST_ERRNO));
			continue;
		}

		/*
		 * Perform functional verification if test
		 * executed without (-f) option.
		 */
		if (STD_FUNCTIONAL_TEST) {
			New_nice = getpriority(PRIO_PROCESS, 0);

			/* Validate functionality of the nice() */
			if (New_nice != (Org_nice + NICEINC)) {
				tst_resm(TFAIL, "nice() failed to modify the "
					 "priority of process");
			} else {
				tst_resm(TPASS, "Functionality of nice(%d) is "
					 "correct", NICEINC);
			}
			Org_nice = New_nice;
		} else {
			tst_resm(TPASS, "call succeeded");
		}
	}

	cleanup();
	tst_exit();

}

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 *  Create a temporary directory and change directory to it.
 *  Get the process id of test process.
 *  Get the current nice value of test process and save it in a file.
 *  Read the nice value from file into a variable.
 */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	Org_nice = getpriority(PRIO_PROCESS, 0);
}

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}