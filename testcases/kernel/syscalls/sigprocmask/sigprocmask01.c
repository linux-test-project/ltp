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
 * Test Name: sigprocmask01
 *
 * Test Description:
 *  Verify that sigprocmask() succeeds to examine and change the calling
 *  process's signal mask.
 *  Also, verify that sigpending() succeeds to store signal mask that are
 *  blocked from delivery and pending for the calling process.
 *
 * Expected Result:
 *  - sigprocmask() should return value 0 on successs and succeed to change
 *    calling process's set of blocked/unblocked signals.
 *  - sigpending() should succeed to store the signal mask of pending signal.
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
 *   	Log the errno and Issue a FAIL message.
 *   Otherwise,
 *   	Verify the Functionality of system call
 *      if successful,
 *      	Issue Functionality-Pass message.
 *      Otherwise,
 *		Issue Functionality-Fail message.
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory created.
 *
 * Usage:  <for command-line>
 *  sigprocmask01 [-c n] [-e] [-f] [-i n] [-I x] [-p x] [-t]
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
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <ucontext.h>

#include "test.h"
#include "usctest.h"

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */
void sig_handler(int sig);	/* signal catching function */

char *TCID = "sigprocmask01";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
int exp_enos[] = { 0 };
int sig_catch = 0;		/* variable to blocked/unblocked signals */

struct sigaction sa_new;	/* struct to hold signal info */
sigset_t sigset;		/* signal set to hold signal lists */
sigset_t sigset2;

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	pid_t my_pid;		/* test process id */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	 /*NOTREACED*/}

	/* Perform global setup for test */
	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		/*
		 * Call sigprocmask() to block (SIGINT) signal
		 * so that, signal will not be delivered to
		 * the test process.
		 */
		TEST(sigprocmask(SIG_BLOCK, &sigset, 0));

		/* Get the process id of test process */
		my_pid = getpid();

		/* Send SIGINT signal to the process */
		kill(my_pid, SIGINT);

		/* check return code of sigprocmask */
		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL,
				 "sigprocmask() Failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			/*
			 * Perform functional verification if test
			 * executed without (-f) option.
			 */
			if (STD_FUNCTIONAL_TEST) {
				/*
				 * Check whether process received the signal.
				 * If yes! signal handler was executed and
				 * incremented 'sig_catch' variable.
				 */
				if (sig_catch) {
					tst_resm(TFAIL, "sigprocmask fails to "
						 "change process's signal mask");
				} else {
					/*
					 * Check whether specified signal
					 * 'SIGINT' is pending for the process.
					 */
					errno = 0;
					if (sigpending(&sigset2) == -1) {
						tst_brkm(TFAIL, cleanup,
							 "blocked signal not "
							 "in pending state, "
							 "error:%d", errno);
					}

					/*
					 * Check whether specified signal
					 * is the member of signal set.
					 */
					errno = 0;
					if (!sigismember(&sigset2, SIGINT)) {
						tst_brkm(TFAIL, cleanup,
							 "sigismember() failed, "
							 "error:%d", errno);
					 /*NOTREACHED*/}

					/*
					 * Invoke sigprocmask() again to
					 * unblock the specified signal.
					 * so that, signal is delivered and
					 * signal handler executed.
					 */
					errno = 0;
					if (sigprocmask(SIG_UNBLOCK,
							&sigset, 0) == -1) {
						tst_brkm(TFAIL, cleanup,
							 "sigprocmask() failed "
							 "to unblock signal, "
							 "error=%d", errno);
					 /*NOTREACHED*/}
					if (sig_catch) {
						tst_resm(TPASS, "Functionality "
							 "of sigprocmask() "
							 "Successful");
					} else {
						tst_resm(TFAIL, "Functionality "
							 "of sigprocmask() "
							 "Failed");
					}
					/* set sig_catch back to 0 */
					sig_catch = 0;
				}
			} else {
				tst_resm(TPASS, "Call succeeded");
			}
		}

		Tst_count++;	/* incr TEST_LOOP counter */
	}			/* End for TEST_LOOPING */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();
	 /*NOTREACHED*/ return 0;

}				/* End main */

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 * Initialise signal set with the list that includes/excludes
 * all system-defined signals.
 * Set the signal handler to catch SIGINT signal.
 * Add the signal SIGINT to the exclude list of system-defined
 * signals for the test process.
 */
void setup()
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/*
	 * Initialise the signal sets with the list that
	 * excludes/includes  all system-defined signals.
	 */
	if (sigemptyset(&sigset) == -1) {
		tst_brkm(TFAIL, cleanup,
			 "sigemptyset() failed, errno=%d : %s",
			 errno, strerror(errno));
	 /*NOTREACHED*/}
	if (sigfillset(&sigset2) == -1) {
		tst_brkm(TFAIL, cleanup,
			 "sigfillset() failed, errno=%d : %s",
			 errno, strerror(errno));
	 /*NOTREACHED*/}

	/* Set the signal handler function to catch the signal */
	sa_new.sa_handler = sig_handler;
	if (sigaction(SIGINT, &sa_new, 0) == -1) {
		tst_brkm(TFAIL, cleanup,
			 "sigaction() failed, errno=%d : %s",
			 errno, strerror(errno));
	 /*NOTREACHED*/}

	/*
	 * Add specified signal (SIGINT) to the signal set
	 * which excludes system-defined signals.
	 */
	if (sigaddset(&sigset, SIGINT) == -1) {
		tst_brkm(TFAIL, cleanup,
			 "sigaddset() failed, errno=%d : %s",
			 errno, strerror(errno));
	 /*NOTREACHED*/}
}				/* End setup() */

/*
 * void
 * sig_handler(int sig) - Signal catching function.
 *   This function gets executed when the signal SIGINT is delivered
 *   to the test process and the signal was trapped by sigaction()
 *   to execute this function.
 *   This function when executed, increments a global variable value
 *   which will be accessed in the test.
 */
void sig_handler(int sig)
{
	/* Increment the sig_catch variable */
	sig_catch++;
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

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */
