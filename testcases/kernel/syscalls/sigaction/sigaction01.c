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
 * NAME
 * 	sigaction01.c
 *
 * DESCRIPTION
 * 	Test some features of sigaction (see below for more details)
 *
 * ALGORITHM
 * 	Use sigaction(2) to set a signal handler for SIGUSR1 with a certain
 * 	set of flags, set a global variable indicating the test case, and
 * 	finally send the signal to ourselves, causing the signal handler to
 * 	run. The signal handler then checks the signal handler to run. The
 * 	signal handler then checks certain conditions based on the test case
 * 	number.
 * 	There are 4 test cases:
 *
 * 	1) Set SA_RESETHAND and SA_SIGINFO. When the handler runs,
 * 	SA_SIGINFO should be set.
 *
 * 	2) Set SA_RESETHAND. When the handler runs, SIGUSR1 should be
 * 	masked (SA_RESETHAND makes sigaction behave as if SA_NODEFER was
 * 	not set).
 *
 * 	3) Same as case #2, but when the handler is established, sa_mask is
 * 	set to include SIGUSR1. Ensure that SIGUSR1 is indeed masked even if
 * 	SA_RESETHAND is set.
 *
 * 	4) A signal generated from an interface or condition that does not
 * 	provide siginfo (such as pthread_kill(3)) should invoke the handler
 * 	with a non-NULL siginfo pointer.
 *
 * USAGE:  <for command-line>
 * sigaction01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	NONE
 */
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include "test.h"

void setup();
void cleanup();

char *TCID = "sigaction01";
int TST_TOTAL = 4;

volatile sig_atomic_t testcase_no;
volatile sig_atomic_t pass;

/*
 * handler()
 *
 * 	A signal handler that understands which test case is currently
 * 	being executed and compares the current conditions to the ones it
 * 	expects (based on the test case number).
 */
void handler(int sig, siginfo_t * sip, void *ucp)
{
	struct sigaction oact;
	int err;
	sigset_t nmask, omask;

	/*
	 * Get sigaction setting
	 */
	err = sigaction(SIGUSR1, NULL, &oact);

	if (err == -1) {
		perror("sigaction");
		return;
	}

	/*
	 * Get current signal mask
	 */
	sigemptyset(&nmask);
	sigemptyset(&omask);
	err = sigprocmask(SIG_BLOCK, &nmask, &omask);
	if (err == -1) {
		perror("sigprocmask");
		tst_resm(TWARN, "sigprocmask() failed");
		return;
	}

	switch (testcase_no) {
	case 1:
		/*
		 * SA_RESETHAND and SA_SIGINFO were set. SA_SIGINFO should
		 * be clear in Linux. In Linux kernel, SA_SIGINFO is not
		 * cleared in psig().
		 */
		if (!(oact.sa_flags & SA_SIGINFO)) {
			tst_resm(TFAIL, "SA_RESETHAND should not "
				 "cause SA_SIGINFO to be cleared, but it was.");
			return;
		}
		if (sip == NULL) {
			tst_resm(TFAIL, "siginfo should not be NULL");
			return;
		}
		tst_resm(TPASS, "SA_RESETHAND did not "
			 "cause SA_SIGINFO to be cleared");
		break;

	case 2:
		/*
		 * In Linux, SA_RESETHAND doesn't imply SA_NODEFER; sig
		 * should not be masked.  The testcase should pass if
		 * SA_NODEFER is not masked, ie. if SA_NODEFER is a member
		 * of the signal list
		 */
		if (sigismember(&omask, sig) == 0) {
			tst_resm(TFAIL, "SA_RESETHAND should cause sig to"
				 "be masked when the handler executes.");
			return;
		}
		tst_resm(TPASS, "SA_RESETHAND was masked when handler "
			 "executed");
		break;

	case 3:
		/*
		 * SA_RESETHAND implies SA_NODEFER unless sa_mask already
		 * included sig.
		 */
		if (!sigismember(&omask, sig)) {
			tst_resm(TFAIL, "sig should continue to be masked"
				 "because sa_mask originally contained sig.");
			return;
		}
		tst_resm(TPASS, "sig has been masked "
			 "because sa_mask originally contained sig");
		break;

	case 4:
		/*
		 * A signal generated from a mechanism that does not provide
		 * siginfo should invoke the handler with a non-NULL siginfo
		 * pointer.
		 */
		if (sip == NULL) {
			tst_resm(TFAIL, "siginfo pointer should not be NULL");
			return;
		}
		tst_resm(TPASS, "siginfo pointer non NULL");
		break;

	default:
		tst_resm(TFAIL, "invalid test case number: %d", testcase_no);
		exit(1);
	}
}

/*
 * set_handler()
 *
 * 	Establish a signal handler for SIGUSR1 with the specified flags and
 * 	signal to mask while the handler executes.
 */
int set_handler(int flags, int sig_to_mask)
{
	struct sigaction sa;

	sa.sa_sigaction = handler;
	sa.sa_flags = flags;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, sig_to_mask);

	TEST(sigaction(SIGUSR1, &sa, NULL));
	if (TEST_RETURN != 0) {
		perror("sigaction");
		tst_resm(TFAIL, "call failed unexpectedly");
		return 1;
	}
	return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup(void)
{

	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup(void)
{

}

int main(int ac, char **av)
{
	int lc;
	int i;
	int test_flags[] = { SA_RESETHAND | SA_SIGINFO, SA_RESETHAND,
		SA_RESETHAND | SA_SIGINFO, SA_RESETHAND | SA_SIGINFO
	};

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset tst_count in case we are looping */
		tst_count = 0;

		testcase_no = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			if (set_handler(test_flags[i], 0) == 0) {
				testcase_no++;
				switch (i) {
				case 0:
				 /*FALLTHROUGH*/ case 1:
					(void)kill(getpid(), SIGUSR1);
					break;
				case 2:
				 /*FALLTHROUGH*/ case 3:
					(void)
					    pthread_kill(pthread_self(),
							 SIGUSR1);
					break;
				default:
					tst_brkm(TBROK, cleanup,
						 "illegal case number");
					break;
				}
			}
		}
	}

	cleanup();
	tst_exit();
}
