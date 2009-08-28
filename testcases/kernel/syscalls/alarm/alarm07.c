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
 * Test Name: alarm07
 *
 * Test Description:
 *  Check the functionality of the alarm() when the time input
 *  parameter is non-zero and the process does a fork.
 *
 * Expected Result:
 *  The alarm request should be cleared in the child process.
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
 *  alarm07 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#include "test.h"
#include "usctest.h"

char *TCID = "alarm07";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
int almreceived = 0;		/* flag to indicate SIGALRM received or not */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */
void sigproc(int sig);		/* signal catching function */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int time_sec = 3;	/* time for which alarm is set */
	int sleep_time = 5;	/* waiting time for the SIGALRM signal */
	pid_t cpid;		/* child process id */

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
		 * 'time_sec' to send SIGALRM to the process.
		 */
		TEST(alarm(time_sec));

		/* Now, fork a child process */
		cpid = FORK_OR_VFORK();
		if (cpid < 0) {
			tst_resm(TFAIL, "fork() fails to create child, "
				 "errno:%d", errno);
		}

		/* Wait for signal SIGALRM to be generated */
		sleep(sleep_time);

		if (STD_FUNCTIONAL_TEST) {
			if (cpid == 0) {	/* Child process */
				/*
				 * For child process if almreceived is 0
				 * means alarm request is cleared.
				 */
				if (almreceived == 0) {
					tst_resm(TPASS, "Functionality of "
						 "alarm(%u) successful",
						 time_sec);
				} else {
					tst_resm(TFAIL, "alarm request not "
						 "cleared in child, "
						 "almreceived:%d", almreceived);
				}
			} else {	/* Parent process */
				/* Wait for child to complete execution */
				wait(0);
			}
		} else {
			tst_resm(TPASS, "call returned %ld", TEST_RETURN);
		}
	}			/* End for TEST_LOOPING */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();

	return 0;
 /*NOTREACHED*/}		/* End main */

/*
 * setup() - performs all ONE TIME setup for this test.
 *  Setup signal handler to catch SIGALRM signal.
 */
void setup()
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

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
