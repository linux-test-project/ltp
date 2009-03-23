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
 * Test Name: sigalstack01
 *
 * Test Description:
 *  Send a signal using the main stack. While executing the signal handler
 *  compare a variable's address lying on the main stack with the stack
 *  boundaries returned by sigaltstack().
 *
 * Expected Result:
 *  sigaltstack() should succeed to get/set signal alternate stack context.
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
 *  sigaltstack01 [-c n] [-e] [-f] [-i n] [-I x] [-p x] [-t]
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
 *  This test should be run by 'super-user' (root) only and must run from
 *  shell which sets up for test.
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <ucontext.h>
#include <errno.h>

#include "test.h"
#include "usctest.h"

char *TCID = "sigaltstack01";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
int exp_enos[] = { 0 };
void *addr, *main_stk;		/* address of main stack for signal */
int got_signal = 0;
pid_t my_pid;			/* test process id */

stack_t sigstk, osigstk;	/* signal stack storing struct. */
struct sigaction act, oact;	/* sigaction() struct. */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */
void sig_handler();		/* signal catching function */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	void *alt_stk;		/* address of alternate stack for signal */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	 /*NOTREACHED*/}

	/* Perform global setup for test */
	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		/* Call sigaltstack() to set up an alternate stack */
		sigstk.ss_size = SIGSTKSZ;
		sigstk.ss_flags = 0;
		TEST(sigaltstack(&sigstk, &osigstk));

		/* check return code of sigaltstack */
		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL,
				 "sigaltstack() Failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			/*
			 * Perform functional verification if test
			 * executed without (-f) option.
			 */
			if (STD_FUNCTIONAL_TEST) {
				/* Set up the signal handler for 'SIGUSR1' */
				act.sa_flags = SA_ONSTACK;
				act.sa_handler = (void (*)())sig_handler;
				if ((sigaction(SIGUSR1, &act, &oact)) == -1) {
					tst_brkm(TFAIL, cleanup, "sigaction() "
						 "fails to trap signal "
						 "delivered on alt. stack, "
						 "error=%d", errno);
				}

				/* Deliver signal onto the alternate stack */
				kill(my_pid, SIGUSR1);

				/* wait till the signal arrives */
				while (!got_signal) ;

				got_signal = 0;
				alt_stk = addr;

				/*
				 * First,
				 * Check that alt_stk is within the
				 * alternate stk boundaries
				 *
				 * Second,
				 * Check that main_stk is outside the
				 * alternate stk boundaries.
				 */
				if ((alt_stk < sigstk.ss_sp) &&
				    (alt_stk > (sigstk.ss_sp + SIGSTKSZ))) {
					tst_resm(TFAIL,
						 "alt. stack is not within the "
						 "alternate stk boundaries");
				} else if ((main_stk >= sigstk.ss_sp) &&
					   (main_stk <=
					    (sigstk.ss_sp + SIGSTKSZ))) {
					tst_resm(TFAIL,
						 "main stk. not outside the "
						 "alt. stack boundaries");
				} else {
					tst_resm(TPASS,
						 "Functionality of "
						 "sigaltstack() successful");
				}
			} else {
				tst_resm(TPASS, "CALL succeeded.");
			}
		}
		Tst_count++;	/* incr. TEST_LOOP counter */
	}			/* End for TEST_LOOPING */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();
	 /*NOTREACHED*/ return 0;

}				/* End main */

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 * Capture SIGUSR1 on the main stack.
 * send the signal 'SIGUSER1' to the process.
 * wait till the signal arrives.
 * Allocate memory for the alternative stack.
 */
void setup()
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Get the process id of test process */
	my_pid = getpid();

	/* Capture SIGUSR1 on the main stack */
	act.sa_handler = (void (*)())sig_handler;
	if ((sigaction(SIGUSR1, &act, &oact)) == -1) {
		tst_brkm(TFAIL, cleanup,
			 "sigaction() fails in setup, errno=%d", errno);
	 /*NOTREACHED*/}

	/* Send the signal to the test process */
	kill(my_pid, SIGUSR1);

	/* Wait till the signal arrives */
	while (!got_signal) ;

	got_signal = 0;
	main_stk = addr;

	/* Allocate memory for the alternate stack */
	if ((sigstk.ss_sp = (void *)malloc(SIGSTKSZ)) == NULL) {
		tst_brkm(TFAIL, cleanup,
			 "could not allocate memory for the alternate stack");
	 /*NOTREACHED*/}
}

/*
 * void
 * sig_handler() - signal catching function.
 *  This functions is called when the signal 'SIGUSR1' is delivered to
 *  the test process and trapped by sigaction().
 *
 *  This function updates 'addr' variable and sets got_signal value.
 */
void sig_handler()
{
	int i;

	addr = &i;
	got_signal = 1;
}

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *  Free the memory allocated for alternate stack.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	free(sigstk.ss_sp);

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */
