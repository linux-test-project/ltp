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
 * Test Name: nice01
 *
 * Test Description:
 *  Verify that root can provide a negative value  to nice()
 *  and hence root can decrease the nice value of the process
 *  using nice() system call
 *
 * Expected Result:
 *  nice() should return value 0 on success and root user should succeed
 *  to decrease the nice value of test process.
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
 *  nice01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
 *  This test should be run by 'super-user' (root) only.
 *
 */
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "test.h"
#include "usctest.h"

#define	NICEINC		-12
#define TEMPFILE	"temp_file"

char *TCID = "nice01";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

int Org_nice;			/* original priority of the test process */
FILE *fp;

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int New_nice;		/* priority of process after nice() */
	int rval;

	/* Parse standard options given to run the test. */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/*
		 * Call nice(2) with an 'incr' parameter set
		 * to a negative value.
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
				tst_resm(TFAIL, "nice() fails to modify the "
					 "priority of process");
			} else {
				tst_resm(TPASS, "Functionality of nice(%d) "
					 "successful", NICEINC);
			}
		} else {
			tst_resm(TPASS, "call succeeded");
		}

		/* return the process to the original priority */
		rval = nice(-NICEINC);

	}

	cleanup();
	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test.
 *  	     Make sure the test process uid is super user.
 *  	     Get the current priority value.
 */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Make sure the calling process is super-user only */
	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Must be ROOT to run this test.");
	}

	TEST_PAUSE;

	Org_nice = getpriority(PRIO_PROCESS, 0);
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *  	       Remove the test directory and testfile created in the setup.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}