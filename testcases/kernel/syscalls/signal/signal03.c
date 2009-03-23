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
 *	signal03.c
 *
 * DESCRIPTION
 *	signal03 - set signals to be ignored
 *
 * ALGORITHM
 *	loop if that option was specified
 *	issue the system call
 *	check the return value
 *	  if return value == -1
 *	    issue a FAIL message, break remaining tests and cleanup
 *	  if we are doing functional testing
 *	    send the signal with kill()
 *	    if we catch the signal
 *	      issue a FAIL message
 *	    else
 *	      issue a PASS message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  signal01 [-c n] [-f] [-i n] [-I x] [-p x] [-t]
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

char *TCID = "signal03";
int TST_TOTAL;
extern int Tst_count;

typedef void (*sighandler_t) (int);

sighandler_t Tret;

int fail = 0;

int siglist[] = { SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGTRAP, SIGABRT, SIGIOT,
	SIGBUS, SIGFPE, SIGUSR1, SIGSEGV, SIGUSR2, SIGPIPE, SIGALRM,
	SIGTERM,
#ifdef SIGSTKFLT
	SIGSTKFLT,
#endif
	SIGCHLD, SIGCONT, SIGTSTP, SIGTTIN,
	SIGTTOU, SIGURG, SIGXCPU, SIGXFSZ, SIGVTALRM, SIGPROF,
	SIGWINCH, SIGIO, SIGPWR, SIGSYS,
#ifdef SIGUNUSED
	SIGUNUSED
#endif
};

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	pid_t pid;
	int i, rval;

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

			errno = 0;
			Tret = signal(siglist[i], SIG_IGN);
			TEST_ERRNO = errno;

			if (Tret == SIG_ERR) {
				tst_brkm(TFAIL, cleanup, "%s call failed - "
					 "errno = %d : %s", TCID,
					 TEST_ERRNO, strerror(TEST_ERRNO));
			 /*NOTREACHED*/}

			if (STD_FUNCTIONAL_TEST) {
				/*
				 * Send the signal.  If the signal is truly set
				 * to be ignored, then the signal handler will
				 * never be invoked and the test will pass.
				 */
				pid = getpid();

				if ((rval = kill(pid, siglist[i])) != 0) {
					tst_brkm(TBROK, cleanup, "call to "
						 "kill failed");
				 /*NOTREACHED*/}

				if (fail == 0) {
					tst_resm(TPASS, "%s call succeeded",
						 TCID);
				} else {
					/* the signal was caught so we fail */
					tst_resm(TFAIL, "signal caught when "
						 "suppose to be ignored");
				}
			} else {
				tst_resm(TPASS, "Call succeeded");
			}
		}
	}

	cleanup();

	 /*NOTREACHED*/ return 0;

}

/*
 * sighandler() - the test fails if we ever get here.
 */
void sighandler(int sig)
{
	fail = 1;
}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void setup(void)
{
	TST_TOTAL = sizeof(siglist) / sizeof(int);
	/* capture signals in our own handler */
	tst_sig(NOFORK, sighandler, cleanup);

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
