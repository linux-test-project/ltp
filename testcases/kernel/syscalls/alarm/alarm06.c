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
 * Test Name: alarm06
 *
 * Test Description:
 *  Check the functionality of the Alarm system call when the time input
 *  parameter is zero.
 *
 * Expected Result:
 *  The previously specified alarm request should be cancelled and the
 *  SIGALRM should not be received.
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
 *  alarm06 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
 *  None.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "test.h"
#include "usctest.h"

char *TCID = "alarm06";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
int almreceived = 0;		/* flag to indicate SIGALRM received or not */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */
void sigproc(int sig);		/* signal handler to catch SIGALRM */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int time_sec1 = 10;	/* time for which 1st alarm is set */
	int time_sec2 = 0;	/* time for which 2nd alarm is set */
	int ret_val1, ret_val2;	/* return values for alarm() calls */
	int sleep_time1 = 5;	/* waiting time for the 1st signal */
	int sleep_time2 = 10;	/* waiting time for the 2nd signal */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	/* Perform global setup for test */
	setup();

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		/*
		 * Call First alarm() with non-zero time parameter
		 * 'time_sec1' to send SIGALRM to the calling process.
		 */
		TEST(alarm(time_sec1));
		ret_val1 = TEST_RETURN;

		/* Wait for signal SIGALARM */
		sleep(sleep_time1);

		/*
		 * Call Second alarm() with zero time parameter
		 * which should cancel our existing alarm.
		 */
		TEST(alarm(time_sec2));
		ret_val2 = TEST_RETURN;

		/* Wait for signal SIGALRM */
		sleep(sleep_time2);

		/*
		 * Check whether the second alarm() call returned
		 * the amount of time (seconds) previously remaining in the
		 * alarm clock of the calling process, and
		 * sigproc() never executed as SIGALRM was not received by the
		 * process, the variable almreceived remains unset.
		 */
		if (STD_FUNCTIONAL_TEST) {
			if ((almreceived == 0) &&
			    (ret_val2 == (time_sec1 - sleep_time1))) {
				tst_resm(TPASS, "Functionality of alarm(%u) "
					 "successful", time_sec2);
			} else {
				tst_resm(TFAIL, "alarm(%u) fails, returned %d, "
					 "almreceived:%d",
					 time_sec2, ret_val2, almreceived);
			}
		} else {
			tst_resm(TPASS, "last call returned %d", ret_val2);
		}
	}			/* End for TEST_LOOPING */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();

	return 0;
 /*NOTREACHED*/}

/*
 * setup() - performs all ONE TIME setup for this test.
 *  Setup signal handler to catch SIGALRM.
 */
void setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Set the signal catching function */
	if (signal(SIGALRM, sigproc) == SIG_ERR) {
		tst_brkm(TFAIL, cleanup,
			 "signal() fails to catch SIGALARM, errno=%d", errno);
	}
}

/*
 * sigproc(int) - This function defines the action that has to be taken
 *	          when the SIGALRM signal is caught.
 *   It also sets the variable which is used to check whether the
 *   alarm system call was successful.
 */
void sigproc(int sig)
{
	almreceived = almreceived + 1;
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

	/* exit with return code appropriate for results */
	tst_exit();
}
