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
 * Test Name: pause02
 *
 * Test Description:
 *  Verify that, pause() returns -1 and sets errno to EINTR after receipt
 *  of a signal which is caught by the calling process. Also, verify that
 *  the calling process will resume execution from the point of suspension.
 *
 * Expected Result:
 *  pause() should fail with return value -1 and set expected errno.
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
 *	if errno set == expected errno
 *		Issue sys call fails with expected return value and errno.
 *	Otherwise,
 *		Issue sys call fails with unexpected errno.
 *   Otherwise,
 *	Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *
 * Usage:  <for command-line>
 *  pause02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
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
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <wait.h>

#include "test.h"
#include "usctest.h"

char *TCID = "pause02";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

int exp_enos[] = { EINTR, 0 };
pid_t cpid;			/* child process id */

void do_child();		/* Function to run in child process */
void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */
void sig_handle(int sig);	/* signal handler for SIGINT */
void kill_handle(int sig);	/* sends SIGKILL for child */

#ifdef UCLINUX
void do_child_uclinux();	/* Setup SIGINT handler then do_child() */
#endif

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int status;		/* child process exit status */
	int rval;		/* return value for wait() */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}
#ifdef UCLINUX
	maybe_run_child(&do_child_uclinux, "");
#endif

	/* Perform global setup for test */
	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

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

		/*
		 * Send the SIGINT signal now, so that child
		 * returns from pause and resumes execution.
		 */
		kill(cpid, SIGINT);

		/* Sleep to ensure the signal sent is effected */
		sleep(1);

		/*
		 * In case pause() doesn't return witin 2 seconds,
		 * set the alarm to send SIGKILL for the child.
		 */
		signal(SIGALRM, kill_handle);
		alarm(2);

		/* wait for child to exit */
		wait(&status);

		TEST_ERROR_LOG(status >> 8);

		/* Reset the timer in case pause() returned */
		alarm(0);

		/*
		 * Verify that, wait() returned due to normal
		 * or abnormal termination of child due to
		 * receipt of signal SIGKILL.
		 * Receipt of SIGKILL indicates that pause()
		 * didn't returned after receipt of SIGINT.
		 */
		if (WTERMSIG(status) == SIGKILL) {
			rval = wait(&status);
			if ((rval == -1) && (errno == ECHILD)) {
				tst_resm(TFAIL, "pause() didn't return "
					 "as expected");
			}
		}
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

	/*
	 * Child resumes execution after receipt of
	 * interrupt signal sent by the parent.
	 */
	TEST_ERROR_LOG(TEST_ERRNO);
	if ((TEST_RETURN == -1) && (TEST_ERRNO == EINTR)) {
		/* pause returned */
		tst_resm(TPASS, "Functionality of pause() " "is correct");
	} else {
		tst_resm(TFAIL, "pause() returned %ld, error=%d",
			 TEST_RETURN, TEST_ERRNO);
	}
	if (TEST_RETURN == -1) {
		exit(TEST_ERRNO);
	} else {
		exit(-1);
	}
}

/*
 * do_child_uclinux()
 */
void do_child_uclinux()
{
	/* Catch SIGINT */
	if (signal(SIGINT, sig_handle) == SIG_ERR) {
		tst_brkm(TBROK, cleanup, "signal() fails to catch SIGINT");
	}

	do_child();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 *	     Set the signal handler to catch SIGINT signal.
 */
void setup()
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Catch SIGINT */
	if (signal(SIGINT, sig_handle) == SIG_ERR) {
		tst_brkm(TBROK, cleanup, "signal() fails to catch SIGINT");
	}
}

/*
 * sig_handle(int sig)
 *    This is the signal handler to handle the SIGINT signal.
 *    When the child receives SIGINT signal while pausing, parent catches
 *    this signal by executing this handler which simply returns.
 */
void sig_handle(int sig)
{
}

/*
 * void
 * kill_handle(int sig)
 *   This is the signal handler to handle the SIGALRM signal.
 *   This handler is executed if and only if pause() doesn't return
 *   in child after sending SIGINT signal. This means child remains
 *   in sleep until kill signal is sent.
 *   Send SIGKILL to child to terminate it.
 */
void kill_handle(int sig)
{
	/* Send SIGKILL to child */
	kill(cpid, SIGKILL);
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
