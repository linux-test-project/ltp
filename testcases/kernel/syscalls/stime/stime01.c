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
 * Test Name: stime01
 *
 * Test Description:
 *  Verify that the system call stime() successfully sets the system's idea
 *  of date and time if invoked by "root" user.
 *
 * Expected Result:
 *  stime() should succeed to set the system data/time to the specified time.
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
 *  stime01 [-c n] [-e] [-f] [-i n] [-I x] [-p x] [-t]
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
 *  This test should be run by 'super-user' (root) only.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>

#include "test.h"
#include "usctest.h"

#define INCR_TIME	10	/* increment in the system's current time */

char *TCID="stime01";		/* Test program identifier.    */
int TST_TOTAL=1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
time_t curr_time;		/* system's current time in seconds */
time_t new_time;		/* system's new time */
time_t tloc;			/* argument var. for time() */
int exp_enos[]={0};

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int
main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	time_t pres_time;	/* system's present time */
    
	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *) NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
		/*NOTREACHED*/
	}

	/* Perform global setup for test */
	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Reset Tst_count in case we are looping. */
		Tst_count=0;

		/* 
		 * Invoke stime(2) to set the system's time
		 * to the specified new_time.
		 */
		TEST(stime(&new_time));

		/* check return code of stime(2) */
		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL, "stime(%ld) Failed, errno=%d : %s",
				 new_time, TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			/*
			 * Perform functional verification if test
			 * executed without (-f) option.
			 */
			if (STD_FUNCTIONAL_TEST) {
				/*
				 * Get the system's current time after call
				 * to stime().
				 */
				if ((pres_time = time(&tloc)) < 0) {
					tst_brkm(TFAIL, cleanup,
						"time() failed to get "
						"system's time after stime, "
						"error=%d", errno);
					/*NOTREACHED*/
				}

				/* Now do the actual verification */
				if ((pres_time != new_time) && 
				   (pres_time != new_time + 1)) {
					tst_resm(TFAIL, "stime() fails to set "
						"system's time");
				} else {
					tst_resm(TPASS, "Functionality of "
						"stime(%ld) successful",
						 new_time);
				}
			} else {
				tst_resm(TPASS, "Call succeeded");
			} 
		}
		Tst_count++;			/* incr TEST_LOOP counter */
	}	/* End for TEST_LOOPING */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();
	/*NOTREACHED*/


  return(0);

}	/* End main */

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 *  Get the current time and system's new time to be set in the test.
 */
void 
setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Check that the test process id is super/root  */
	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Must be super/root for this test!");
		tst_exit();
		/*NOTREACHED*/
	}

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Get the current time */
	if ((curr_time = time(&tloc)) < 0) {
		tst_brkm(TBROK, cleanup,
			 "time() failed to get current time, errno=%d",
			 errno);
		/*NOTREACHED*/
	}

	/* Get the system's new time */
	new_time = curr_time + INCR_TIME;
}	/* End setup() */

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 */
void 
cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}	/* End cleanup() */
