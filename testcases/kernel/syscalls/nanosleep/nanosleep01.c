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
 * Test Name: nanosleep01
 *
 * Test Description:
 *  Verify that nanosleep() will be successful to suspend the execution
 *  of a process for a specified time.
 *
 * Expected Result:
 *  nanosleep() should return with value 0 and the process should be
 *  suspended for time specified by timespec structure.
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
 *   	Issue a FAIL message.
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
 *  nanosleep01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>

#include "test.h"
#include "usctest.h"

char *TCID="nanosleep01";	/* Test program identifier.    */
int TST_TOTAL=1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

struct timespec timereq;	/* time struct. buffer for nanosleep() */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int
main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	pid_t cpid;		/* Child process id */
	time_t otime;		/* time before child execution suspended */
	time_t ntime;		/* time after child resumes execution */
	int status;		/* child exit status */
    
	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *) NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	/* Perform global setup for test */
	setup();

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* Reset Tst_count in case we are looping. */
		Tst_count=0;

		/*
		 * Creat a child process and suspend it till
		 * time specified by timespec struct element
		 * time_t tv_sec.
		 */
		cpid = fork();
		if (cpid == -1) {
			tst_brkm(TBROK, cleanup, "fork() failed");
		}

		if (cpid == 0) {		/* Child process */
			/* Note down the current time */
			time(&otime);
			/* 
			 * Call nanosleep() to suspend child process
			 * for specified time.
			 */
			TEST(nanosleep(&timereq, NULL));

			/* time after child resumes execution */
			time(&ntime);

			/* check return code of nanosleep() */
			if (TEST_RETURN == -1) {
				tst_resm(TFAIL,
					 "nanosleep() failed, errno=%d : %s",
					 TEST_ERRNO, strerror(TEST_ERRNO));
				continue;
			}

			/*
		 	 * Perform functional verification if test
		 	 * executed without (-f) option.
		 	 */
			if (STD_FUNCTIONAL_TEST) {
				/*
				 * Verify whether child execution was 
				 * actually suspended.
				 */
				if ((ntime - otime) != timereq.tv_sec) {
					tst_resm(TFAIL, "Child execution not "
						 "suspended for %d seconds",
						 timereq.tv_sec);
				} else {
					tst_resm(TPASS, "nanosleep "
						 "functionality is correct");
				}
			} else {
				tst_resm(TPASS, "call succeeded");
			}
		} else {			/* parent process */
			/* let the child carry on */
			exit(0);
		}
	}	/* End for TEST_LOOPING */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();

	return 0;
}	/* End main */

/*
 * setup() - performs all ONE TIME setup for this test.
 *  	     Initialize time structure elements.
 */
void 
setup()
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Initialise time variables which used to suspend child execution */
	timereq.tv_sec = 2;
	timereq.tv_nsec = 9999;
}


/*
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
}
