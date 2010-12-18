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
 * Test Name: getpriority01
 *
 * Test Description:
 *  Verify that getpriority() succeeds get the scheduling priority of
 *  the current process, process group or user.
 *
 * Expected Result:
 *  getpriority() should return the highest priority of the test process.
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
 *  getpriority01 [-c n] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS:
 *  None.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "test.h"
#include "usctest.h"

char *TCID = "getpriority01";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

void setup();			/* setup function for the test */
void cleanup();			/* cleanup function for the test */

int prio_which[] = { PRIO_PROCESS, PRIO_PGRP, PRIO_USER };

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int ind;
	int which;		/* scheduling priority category */

	TST_TOTAL = sizeof(prio_which) / sizeof(int);

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, NULL, NULL);
	if (msg != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		for (ind = 0; ind < TST_TOTAL; ind++) {
			which = prio_which[ind];

			/*
			 * Invoke getpriority with the specified
			 * 'which' argument for the calling process.
			 */
			TEST(getpriority(which, 0));

			if (TEST_RETURN < 0 && TEST_ERRNO != 0) {
				tst_resm(TFAIL, "getpriority(%d, 0) "
					 "Failed, errno=%d : %s",
					 which, TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TPASS, "getpriority(%d, 0) returned "
					 "%ld priority value",
					 which, TEST_RETURN);
			}
		}
	}

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