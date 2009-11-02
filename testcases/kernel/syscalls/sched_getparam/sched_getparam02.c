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
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */
/**********************************************************
 *
 *    TEST IDENTIFIER	: sched_getparam02
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Get scheduling parametes for parent process
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
 *	Verifies functionality of sched_getparam() for a process other than
 *	current process (ie, pid != 0). Here we get the scheduling parameters
 *	for parent process.
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	 fork a child
 *
 *	 CHILD:
 * 	  Gets the scheduling parameters for parent process
 *	  If successfull,
 *		TEST passed
 *	  else
 *		TEST failed.
 *
 *	 PARENT:
 *	  wait for child to finish
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  sched_getparam02 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
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
#include <sys/wait.h>
#include <stdlib.h>
#include "test.h"
#include "usctest.h"

static void setup();
static void cleanup();

char *TCID = "sched_getparam02";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

static struct sched_param param;

int main(int ac, char **av)
{

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int status;
	pid_t child_pid;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL))
	    != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	/* perform global setup for test */
	setup();

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		switch (child_pid = FORK_OR_VFORK()) {

		case -1:
			/* fork() failed */
			tst_resm(TFAIL, "fork() failed");
			continue;

		case 0:
			/* Child */
			param.sched_priority = 100;

			/*
			 * Call sched_getparam(2) with pid = getppid() sothat
			 * it will get the scheduling parameters for parent
			 * process
			 */
			TEST(sched_getparam(getppid(), &param));

			/*
			 * Check return code & priority. For normal process,
			 * scheduling policy is SCHED_OTHER. For this
			 * scheduling policy, only allowed priority value is 0.
			 * So we should get 0 for priority value
			 */
			if ((TEST_RETURN == 0) && (param.sched_priority == 0)) {
				exit(0);
			} else {
				tst_resm(TWARN, "sched_getparam()"
					 "returned %ld, errno = %d : %s;"
					 " returned process priority value"
					 " is %d", TEST_RETURN, TEST_ERRNO,
					 strerror(TEST_ERRNO),
					 param.sched_priority);
				exit(1);
			}

		default:
			/* Parent */
			if ((waitpid(child_pid, &status, 0)) < 0) {
				tst_resm(TFAIL, "wait() failed");
				continue;
			}
			if ((WIFEXITED(status)) && (WEXITSTATUS(status) == 0)) {
				tst_resm(TPASS, "Test Passed");
			} else {
				tst_resm(TFAIL, "Test Failed");
			}
		}

	}			/* End for TEST_LOOPING */

	/* cleanup and exit */
	cleanup();

	 /*NOTREACHED*/ return 0;

}				/* End main */

/* setup() - performs all ONE TIME setup for this test */
void setup()
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

}				/* End setup() */

/*
 *cleanup() -  performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
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
}				/* End cleanup() */
