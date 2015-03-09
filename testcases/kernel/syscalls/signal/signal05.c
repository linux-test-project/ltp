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
 *	signal05.c
 *
 * DESCRIPTION
 *	signal05 - set the signal handler to our own function
 *
 * ALGORITHM
 *	loop if that option was specified
 *	issue the system call
 *	check the return value
 *	  if return value == -1
 *	    issue a FAIL message, break remaining tests and cleanup
 *	  if we are doing functional testing
 *	    send the signal with kill()
 *	    if we catch the signal in the signal handler
 *	      issue a PASS message
 *	    else
 *	      issue a FAIL message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  signal05 [-c n] [-f] [-i n] [-I x] [-p x] [-t]
 *     where,  -c n : Run n copies concurrently.
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
 * Restrictions
 *	none
 */

#include "test.h"

#include <errno.h>
#include <signal.h>

void cleanup(void);
void setup(void);
void sighandler(int);

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

char *TCID = "signal05";
int TST_TOTAL = ARRAY_SIZE(siglist);

typedef void (*sighandler_t) (int);

sighandler_t Tret;

int pass = 0;

int main(int ac, char **av)
{
	int lc;
	pid_t pid;
	int i, rval;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		/*
		 * loop through the list of signals and test each one
		 */
		for (i = 0; i < TST_TOTAL; i++) {

			errno = 0;
			Tret = signal(siglist[i], &sighandler);
			TEST_ERRNO = errno;

			if (Tret == SIG_ERR) {
				tst_resm(TFAIL, "%s call failed - errno = %d "
					 ": %s", TCID, TEST_ERRNO,
					 strerror(TEST_ERRNO));
				continue;
			}

			/*
			 * Send the signals and make sure they are
			 * handled in our handler.
			 */
			pid = getpid();

			if ((rval = kill(pid, siglist[i])) != 0) {
				tst_brkm(TBROK, cleanup,
					 "call to kill failed");
			}

			if (siglist[i] == pass) {
				tst_resm(TPASS,
					 "%s call succeeded", TCID);
			} else {
				tst_resm(TFAIL,
					 "received unexpected signal");
			}
		}
	}

	cleanup();
	tst_exit();
}

/*
 * sighandler() - handle the signals
 */
void sighandler(int sig)
{
	/* set the global pass variable = sig */
	pass = sig;
}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void setup(void)
{
	TEST_PAUSE;
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{

}
