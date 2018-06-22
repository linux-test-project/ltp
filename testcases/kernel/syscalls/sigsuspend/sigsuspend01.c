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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test Name: sigsuspend01
 *
 * Test Description:
 *  Verify that sigsuspend() succeeds to change process's current signal
 *  mask with the specified signal mask and suspends the process execution
 *  until the delivery of a signal.
 *
 * Expected Result:
 *  sigsuspend() should return after the execution of signal catching
 *  function and the previous signal mask should be restored.
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
 *  sigsuspend01 [-c n] [-e] [-f] [-i n] [-I x] [-p x] [-t]
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

char *TCID = "sigsuspend01";
int TST_TOTAL = 1;

struct sigaction sa_new;	/* struct to hold signal info */
sigset_t signalset;		/* signal set to hold signal lists */
sigset_t sigset1;
sigset_t sigset2;

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */
void sig_handler(int sig);	/* signal catching function */

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/* Set the alarm timer */
		alarm(5);

		/*
		 * Call sigsuspend() to replace current signal mask
		 * of the process and suspend process execution till
		 * receipt of a signal 'SIGALRM'.
		 */
		TEST(sigsuspend(&signalset));

		/* Reset the alarm timer */
		alarm(0);

		if ((TEST_RETURN == -1) && (TEST_ERRNO == EINTR)) {
			if (sigprocmask(SIG_UNBLOCK, 0, &sigset2) == -1) {
				tst_resm(TFAIL, "sigprocmask() Failed "
					 "to get previous signal mask "
					 "of process");
			} else if (memcmp(&sigset1, &sigset2,
				   sizeof(unsigned long))) {
				tst_resm(TFAIL, "sigsuspend failed to "
					 "preserve signal mask");
			} else {
				tst_resm(TPASS, "Functionality of "
					 "sigsuspend() successful");
			}
		} else {
			tst_resm(TFAIL | TTERRNO,
				 "sigsuspend() returned value %ld",
				 TEST_RETURN);
		}

		tst_count++;	/* incr TEST_LOOP counter */
	}

	cleanup();
	tst_exit();
}

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 * Initialise signal set with the list that includes/excludes
 * all system-defined signals.
 * Set the signal handler to catch SIGALRM signal.
 * Get the current signal mask of test process using sigprocmask().
 */
void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/*
	 * Initialise the signal sets with the list that
	 * excludes/includes  all system-defined signals.
	 */
	if (sigemptyset(&signalset) == -1) {
		tst_brkm(TFAIL, cleanup,
			 "sigemptyset() failed, errno=%d : %s",
			 errno, strerror(errno));
	}
	if (sigfillset(&sigset2) == -1) {
		tst_brkm(TFAIL, cleanup,
			 "sigfillset() failed, errno=%d : %s",
			 errno, strerror(errno));
	}

	/* Set the signal handler function to catch the signal */
	sa_new.sa_handler = sig_handler;
	if (sigaction(SIGALRM, &sa_new, 0) == -1) {
		tst_brkm(TFAIL, cleanup,
			 "sigaction() failed, errno=%d : %s",
			 errno, strerror(errno));
	}

	/* Read the test process's current signal mask. */
	if (sigprocmask(SIG_UNBLOCK, 0, &sigset1) == -1) {
		tst_brkm(TFAIL, cleanup,
			 "sigprocmask() Failed, errno=%d : %s",
			 errno, strerror(errno));
	}
}

/*
 * void
 * sig_handler(int sig) - Signal catching function.
 *   This function gets executed when the signal SIGALRM is delivered
 *   to the test process after the expiry of alarm time and the signal was
 *   trapped by sigaction() to execute this function.
 *
 *   This function simply returns without doing anything.
 */
void sig_handler(int sig)
{
}

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 */
void cleanup(void)
{

}
