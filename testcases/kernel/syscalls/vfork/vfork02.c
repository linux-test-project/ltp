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
 * Test Name: vfork02
 *
 * Test Description:
 *  Fork a process using vfork() and verify that, the pending signals in
 *  the parent are not pending in the child process.
 * $
 * Expected Result:
 *  The signal which is pending in the parent should not be pending in the
 *  child process.
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
 *  vfork02 [-c n] [-e] [-f] [-i n] [-I x] [-p x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions:
 *  None.
 *
 */
#define _GNU_SOURCE 1

#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <wait.h>

#include "test.h"
#include "usctest.h"

char *TCID = "vfork02";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
int exp_enos[] = { 0 };

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */
void sig_handler();		/* signal catching function */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	pid_t cpid;		/* process id of the child process */
	int exit_status;	/* exit status of child process */
	sigset_t PendSig;	/* variable to hold pending signal */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	/* Perform global setup for test */
	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		/*
		 * Call vfork(2) to create a child process without
		 * fully copying the address space of parent.
		 */
		TEST(vfork());

		/* check return code of vfork() */
		if ((cpid = TEST_RETURN) == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL, "vfork() Failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else if (cpid == 0) {	/* Child process */
			/*
			 * Perform functional verification if test
			 * executed without (-f) option.
			 */
			if (STD_FUNCTIONAL_TEST) {
				/*
				 * Check whether the pending signal SIGUSR1
				 * in the parent is also pending in the child
				 * process by storing it in a variable.
				 */
				if (sigpending(&PendSig) == -1) {
					tst_resm(TFAIL, "sigpending function "
						 "failed in child");
					_exit(1);
				}

				/* Check if SIGUSR1 is pending in child */
				if (sigismember(&PendSig, SIGUSR1) != 0) {
					tst_resm(TFAIL, "SIGUSR1 also pending "
						 "in child process");
					_exit(1);
				}

				/*
				 * Exit with normal exit code if everything
				 * fine
				 */
				_exit(0);
			}
		} else {	/* parent process */
			/*
			 * Let the parent process wait till child completes
			 * its execution.
			 */
			wait(&exit_status);

			/* Check for the exit status of child process */
			if (WEXITSTATUS(exit_status) == 0) {
				tst_resm(TPASS, "Call to vfork() "
					 "successful");
			} else if (WEXITSTATUS(exit_status) == 1) {
				tst_resm(TFAIL,
					 "Child process exited abnormally");
			}
		}
		Tst_count++;	/* incr. TEST_LOOP counter */
	}			/* End for TEST_LOOPING */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();

	return 0;
}				/* End main */

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 *   This function installs signal handler for SIGUSR1, puts signal SIGUSR1
 *   on hold and then sends the signal SIGUSR1 to itself so that it is in
 *   pending state.
 */
void setup()
{
	sigset_t PendSig;	/* variable to hold pending signal */
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Install the signal handler */
	if (signal(SIGUSR1, sig_handler) == SIG_ERR) {
		tst_brkm(TBROK, cleanup, "Fails to catch the signal SIGUSR1");
	}

	/* Hold the signal SIGUSR1 */
	if (sighold(SIGUSR1) == -1) {
		tst_brkm(TBROK, cleanup,
			 "sighold failed to hold the signal SIGUSR1");
	}

	/* Send the signal SIGUSR1 to itself so that SIGUSR1 is pending */
	if (kill(getpid(), SIGUSR1) == -1) {
		tst_brkm(TBROK, cleanup,
			 "Fails to send the signal to the parent process");
	}

	/* If SIGUSR1 is not pending in the parent, fail */
	if (sigpending(&PendSig) == -1) {
		tst_brkm(TBROK, cleanup,
			 "sigpending function failed in parent");
	}

	/* Check if SIGUSR1 is pending in parent */
	if (sigismember(&PendSig, SIGUSR1) != 1) {
		tst_brkm(TBROK, cleanup,
			 "SIGUSR1 signal is not pending in parent");
	}
}				/* End setup() */

/*
 * void
 * sig_handler() - signal catching function for 'SIGUSR1' signal.
 *		 $
 *   This is a null function and used only to catch the above signal
 *   generated in parent process.
 */
void sig_handler()
{
}

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *  Release the signal 'SIGUSR1'  if still in pending state.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Release the signal 'SIGUSR1' if in pending state */
	if (sigrelse(SIGUSR1) == -1) {
		tst_brkm(TBROK, NULL, "Failed to release 'SIGUSR1' in cleanup");
	}

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */
