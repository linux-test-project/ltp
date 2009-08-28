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
 * Test Name: pause03
 *
 * Test Description:
 *  Verify that a process is no longer accessible on receipt of SIGKILL
 *  signal after being suspended by pause().
 *
 * Expected Result:
 *  pause() does not return due to receipt of SIGKILL signal and specified
 *  process should be terminated.
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
 *	Log the errno and Issue a FAIL message.
 *   Otherwise,
 *	Verify the Functionality of system call
 *      if successful,
 *		Issue Functionality-Pass message.
 *      Otherwise,
 *		Issue Functionality-Fail message.
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *
 * Usage:  <for command-line>
 *  pause03 [-c n] [-i n] [-I x] [-P x] [-t]
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
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <wait.h>

#include "test.h"
#include "usctest.h"

int cflag;			/* flag to indicate child process status */
pid_t cpid;			/* child process id */

char *TCID = "pause03";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

void do_child();		/* Function to run in child process */
void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */
void sig_handle(int sig);	/* signal handler for SIGCLD */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int status;		/* child process exit status */
	int ret_val;		/* return value for wait() */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}
#ifdef UCLINUX
	maybe_run_child(&do_child, "");
#endif

	/* Perform global setup for test */
	setup();

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		/* Creat a new process using fork() */
		if ((cpid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork() failed");
		}

		if (cpid == 0) {	/* Child process */
#ifdef UCLINUX
			if (self_exec(av[0], "") < 0) {
				tst_brkm(TBROK, cleanup, "self_exec failed");
			}
#else
			do_child();
#endif
		}

		/* Parent process */
		/* sleep to ensure the child executes */
		sleep(1);

		/* Check for the value of cflag */
		if (cflag == 1) {
			/*
			 * Indicates that child terminated
			 * before receipt of SIGKILL signal.
			 */
			tst_brkm(TFAIL, cleanup,
				 "Child exited before SIGKILL signal");
		}

		/* Send the SIGKILL signal now */
		kill(cpid, SIGKILL);

		/* sleep to ensure the signal sent is effected */
		sleep(1);

		/* Verify again the value of cflag */
		if (cflag == 0) {
			/* Child still exists */
			tst_resm(TFAIL, "Child still exists, "
				 "pause() still active");
			cleanup();
		}

		ret_val = wait(&status);

		/*
		 * Verify that wait returned after child process termination
		 * due to receipt of SIGKILL signal.
		 */
		if (WTERMSIG(status) == SIGKILL) {
			ret_val = wait(&status);
			if ((ret_val == -1) && (errno == ECHILD)) {
				/*
				 * Child is no longer accessible and pause()
				 * functionality is successful.
				 */
				tst_resm(TPASS, "Functionality of "
					 "pause() is correct");
			} else {
				tst_resm(TFAIL, "wait() failed due to "
					 "unkown reason, ret_val=%d, "
					 "errno=%d", ret_val, errno);
			}
		} else {
			tst_resm(TFAIL, "Child terminated not due to "
				 "SIGKILL, errno = %d", errno);
		}

		/* reset cflag in case we are looping */
		cflag = 0;

	}			/* End for TEST_LOOPING */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();

	return 0;
}				/* End main */

/*
 * do_child()
 */
void do_child()
{
	/* Suspend the child using pause() */
	TEST(pause());

	/* print the message if pause() returned */
	tst_resm(TFAIL, "Unexpected return from pause()");
	/* Loop infinitely */
	while (1) ;
}

/*
 * setup() - performs all ONE TIME setup for this test.
 *	     Setup signal handler to catch SIGCLD signal.
 */
void setup()
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Initialise cflag */
	cflag = 0;

	/* Catch SIGCLD */
	signal(SIGCLD, sig_handle);
}

/*
 * sig_handle(int sig)
 *    This is the signal handler to handle the SIGCLD signal.
 *    When the child terminates and the parent gets the SIGCLD signal
 *    the handler gets executed and then the cflag variable is set to
 *    indicate the child has terminated.
 */
void sig_handle(int sig)
{
	/* Set the cflag variable */
	cflag = 1;
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

	/* Cleanup the child if still active */
	kill(cpid, SIGKILL);

	/* exit with return code appropriate for results */
	tst_exit();
}
