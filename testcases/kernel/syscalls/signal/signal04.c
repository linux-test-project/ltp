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
 * NAME
 *	signal04.c
 *
 * DESCRIPTION
 *	signal04 - restore signals to default behavior
 *
 * ALGORITHM
 *	loop if that option was specified
 *	for each signal in siglist[]
 *	  set the signal handler to our own and save the return value
 *	  issue the signal system call to restore the default behavior
 *	  check the return value
 *	  if return value == -1
 *	    issue a FAIL message, break remaining tests and cleanup
 *	  if we are doing functional testing
 *	    set the signal handler to our own again and save second return value
 *	    if the first return value matches the second return value
 *	      issue a PASS message
 *	    else
 *	      issue a FAIL message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  signal04 [-c n] [-f] [-i n] [-I x] [-p x] [-t]
 *	where,  -c n : Run n copies concurrently.
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
 * Restrictions
 *	none
 */

#include "test.h"
#include "usctest.h"

#include <errno.h>
#include <signal.h>

void cleanup(void);
void setup(void);
void sighandler(int);

char *TCID = "signal04";
extern int Tst_count;

int siglist[] = { SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGTRAP, SIGABRT,
	SIGBUS, SIGFPE, SIGUSR1, SIGSEGV, SIGUSR2, SIGPIPE, SIGALRM,
	SIGTERM, SIGCHLD, SIGCONT, SIGTSTP, SIGTTIN,
	SIGTTOU, SIGURG, SIGXCPU, SIGXFSZ, SIGVTALRM, SIGPROF,
	SIGWINCH, SIGIO, SIGPWR, SIGSYS
};

int TST_TOTAL = sizeof(siglist) / sizeof(int);

typedef void (*sighandler_t) (int);

sighandler_t Tret;

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int i;
	sighandler_t rval, first;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/*
		 * loop through the list of signals and test each one
		 */
		for (i = 0; i < TST_TOTAL; i++) {

			/* First reset the signal to the default
			   action to clear out any pre-test
			   execution settings */
			signal(siglist[i], SIG_DFL);

			/* then set the handler to our own handler */
			if ((rval = signal(siglist[i], &sighandler)) == SIG_ERR) {
				tst_brkm(TBROK, cleanup, "initial signal call"
					 " failed");
			 /*NOTREACHED*/}

			/* store the return value */
			first = rval;

			/* restore the default signal action */
			errno = 0;
			Tret = signal(siglist[i], SIG_DFL);
			TEST_ERRNO = errno;

			if (Tret == SIG_ERR) {
				tst_brkm(TFAIL, cleanup, "%s call failed - "
					 "errno = %d : %s", TCID, TEST_ERRNO,
					 strerror(TEST_ERRNO));
			 /*NOTREACHED*/}

			if (STD_FUNCTIONAL_TEST) {
				/* now set the handler back to our own */
				if ((rval = signal(siglist[i], &sighandler))
				    == SIG_ERR) {
					tst_brkm(TBROK, cleanup, "initial "
						 "signal call failed");
				 /*NOTREACHED*/}

				/*
				 * the first return value should equal the
				 * second one.
				 */
				if (rval == first) {
					tst_resm(TPASS, "%s call succeeded "
						 "received %p.", TCID, rval);
				} else {
					tst_brkm(TFAIL, cleanup, "return "
						 "values for signal(%d) don't "
						 "match. Got %p, expected %p.",
						 siglist[i], rval, first);
				 /*NOTREACHED*/}
			} else {
				tst_resm(TPASS, "Call of signal(%d) succeeded",
					 siglist[i]);
			}
		}
	}

	cleanup();
	 /*NOTREACHED*/ return 0;

}

/*
 * sighandler() - handle the signals
 */
void sighandler(int sig)
{
}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void setup(void)
{
	/* Pause if that option was specified */
	TEST_PAUSE;
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}
