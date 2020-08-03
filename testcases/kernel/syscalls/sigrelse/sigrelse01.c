/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */
/* $Id: sigrelse01.c,v 1.14 2009/08/28 14:10:16 vapier Exp $ */
/*****************************************************************************
 * OS Test - Silicon Graphics, Inc.  Eagan, Minnesota
 *
 * TEST IDENTIFIER : sigrelse01 Releasing held signals.
 *
 * PARENT DOCUMENT : sgrtds01  sigrelse system call
 *
 * AUTHOR          : Bob Clark
 *		   : Rewrote 12/92 by Richard Logan
 *
 * CO-PILOT        : Dave Baumgartner
 *
 * DATE STARTED    : 10/08/86
 *
 * TEST ITEMS
 *
 * 	1. sigrelse turns on the receipt of signals held by sighold.
 *
 * SPECIAL PROCEDURAL REQUIRMENTS
 * 	None
 *
 * DETAILED DESCRIPTION
 * 	set up pipe for parent/child communications
 * 	fork off a child process
 *
 * 	parent():
 * 		set up for unexpected signals
 * 		wait for child to send ready message over pipe
 * 		send all catchable signals to child process
 *		send alarm signal to speed up timeout
 * 		wait for child to terminate and check exit value
 *
 * 		if exit value is EXIT_OK
 * 		  get message from pipe (contains array of signal counters)
 * 		  loop through array of signal counters and record any
 * 			signals which were not caught once.
 * 		  record PASS or FAIL depending on what was found in the array.
 *
 * 		else if exit is SIG_CAUGHT then BROK (signal caught
 *		  before released)
 * 		else if exit is WRITE_BROK then BROK (write() to pipe failed)
 * 		else if exit is HANDLE_ERR then BROK (error in child's
 *		  signal handler)
 * 		else unexpected exit value - BROK
 *
 * 	child():
 * 	  phase 1:
 * 		set up to catch all catchable signals (exit SIG_CAUGHT
 *		  if caught)
 * 		hold each signal with sighold()
 * 		send parent ready message if setup went ok.
 * 		wait for signals to arrive - timeout if they don't
 *
 * 	  phase 2:
 * 		release each signal and wait a second for the handler to
 *		  catch it.
 * 		(the handler will record each signal it catches in an array
 * 		and exit HANDLE_ERR if an error occurs)
 *
 * 		send array of counters back to parent for processing.
 * 		exit EXIT_OK
 * NOTES
 *	since child is executing system calls under test, no
 *	system call times are printed.
 *
***************************************************************************/

#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "test.h"
#include "safe_macros.h"

#ifdef __linux__
/* glibc2.2 definition needs -D_XOPEN_SOURCE, which breaks other things. */
extern int sighold(int __sig);
extern int sigrelse(int __sig);
#endif

void setup(void);
void cleanup(void);
static void verify_sigrelse(void);
static int setup_sigs(void);
static void handler(int sig);
static void wait_a_while(void);
int choose_sig(int sig);

#define TRUE  1
#define FALSE 0

#ifndef DEBUG
#define DEBUG 0
#endif
#define TIMEOUT 30		/* time (sec) used in the alarm calls */

int TST_TOTAL = 1;		/* number of test items */

char *TCID = "sigrelse01";	/* test case identifier */
static int pid;			/* process id of child */
static int phase;		/* flag for phase1 or phase2 of */
				/* signal handler */
static int sig_caught;		/* flag TRUE if signal caught */
				/* (see wait_a_while ()) */

/* ensure that NUMSIGS is defined. */
#ifndef NUMSIGS
#define NUMSIGS NSIG
#endif

#define NUMTSTSIGS 32
/* array of counters for signals caught by handler() */
static int sig_array[NUMTSTSIGS];

void hold_signals(void)
{
	int sig;		/* current signal number */
	int rv;			/* function return value */

        if (setup_sigs() < 0) {
                /* an error occured - exit from test  */
                tst_brkm(TBROK, cleanup, "Failed to setup sugnals");

        } else {
                /* all set up to catch signals, now hold them */

                for (sig = 1; sig < NUMTSTSIGS; sig++) {
                        if (choose_sig(sig)) {
                                if ((rv = sighold(sig)) != 0) {
                                        /* THEY say sighold ALWAYS returns 0 */
                                        tst_brkm(TBROK|TERRNO, cleanup, "Failed to do sighold, returned: %d", rv);
                                }
                        }
                }

        }

}

void release_signals(void)
{
	int sig;		/* current signal number */
	int rv;			/* function return value */
	for (sig = 1; sig < NUMTSTSIGS; sig++) {
        	if (choose_sig(sig)) {
			/* all set up, release and catch a signal */

			sig_caught = FALSE;     /* handler sets it to TRUE when caught */
			if ((rv = sigrelse(sig)) != 0) {
				/* THEY say sigrelse ALWAYS returns 0 */
				tst_brkm(TBROK|TERRNO, cleanup, "sigrelse did not return 0. rv:%d", rv);
			}

			/* give signal handler some time to process signal */
			wait_a_while();
        	}

        }                       /* endfor */

}

/***********************************************************************
 *   M A I N
 ***********************************************************************/
int main(int argc, char **argv)
{
	int lc;

	/* gcc -Wall complains about sig_caught not being ref'd because of the
	   external declarations. */
	sig_caught = FALSE;

	/*
	 * parse standard options
	 */
	tst_parse_opts(argc, argv, NULL, NULL);

	/*
	 * perform global setup for test
	 */
	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;
		verify_sigrelse();
	}

	cleanup();
	tst_exit();

}				/* end main */


/****************************************************************************
 * verify_sigrelse() : verifys the sigrelse functionality
 ***************************************************************************/
static void verify_sigrelse(void)
{
	int sig;		/* current signal number */
	int fail = FALSE;	/* flag indicating test item failure */
	int caught_sigs;

	phase = 1;
	hold_signals();

	/*
	 * send signals to child and see if it holds them
	 */

	for (sig = 1; sig < NUMTSTSIGS; sig++) {
		if (choose_sig(sig)) {
			if (kill(pid, sig) < 0) {
				if (errno == ESRCH) {
					if (kill(pid, SIGTERM) < 0)
						tst_brkm(TBROK | TERRNO, cleanup,
							 "kill(%d, %d) and kill(%d, SIGTERM) failed",
							 pid, sig, pid);
					else
						tst_brkm(TBROK | TERRNO, cleanup,
							 "kill(%d, %d) failed, but kill(%d, SIGTERM) worked",
							 pid, sig, pid);
				} else
					tst_brkm(TBROK | TERRNO, cleanup,
						 "kill(%d, %d) failed", pid,
						 sig);
			}
		}
	}

	phase = 2;
	release_signals();

	caught_sigs = 0;
	for (sig = 1; sig < NUMTSTSIGS; sig++) {
		if (choose_sig(sig)) {
			if (sig_array[sig] != 1) {
				/* sig was not caught or caught too many times */
				tst_brkm(TBROK, cleanup, "\tsignal %d caught %d times (expected 1).\n",
						      sig, sig_array[sig]);
				fail = TRUE;
			} else {
				caught_sigs++;
			}
		}
	}			/* endfor */

	if (fail == TRUE)
		tst_resm(TFAIL, "%s", "Signal caught multiple times");
	else
		tst_resm(TPASS, "sigrelse() released all %d signals under test.",
				 caught_sigs);

}			/* end of verify_sigrelse*/

/*****************************************************************************
 *  setup_sigs() : set child up to catch all signals.  If there is
 *       trouble, write message in mesg and return -1, else return 0.
 *       The signal handler has two functions depending on which phase
 *       of the test we are in.  The first section is executed after the
 *       signals have been held (should not ever be used).  The second
 *       section is executed after the signals have been released (should
 *       be executed for each signal).
 ****************************************************************************/
static int setup_sigs(void)
{
	int sig;

	/* set up signal handler routine */
	for (sig = 1; sig < NUMTSTSIGS; sig++) {
		if (choose_sig(sig)) {
			if (signal(sig, handler) == SIG_ERR) {
				/* set up mesg to send back to parent */
				tst_brkm(TBROK,cleanup,
					      "signal() failed for signal %d. error:%d %s.",
					      sig, errno, strerror(errno));
			}
		}
	}
	return 0;

}				/* end of setup_sigs  */

/*****************************************************************************
 *  handler() : child's interrupt handler for all signals.  The phase variable
 *      is set in the child process indicating what action is to be taken.
 *    The phase 1 section will be run if the child process catches a signal
 *      after the signal has been held resulting in a test item BROK.
 *      The parent detects this situation by a child exit value of SIG_CAUGHT.
 *    The phase 2 section will be run if the child process catches a
 *      signal after the signal has been released.  All signals must be
 *      caught in order for a PASS.
 ****************************************************************************/
static void handler(int sig)
{
	static int s = 0;	/* semaphore so that we don't handle 2 */
	/* sigs at once */

#if DEBUG > 1
	printf("child: handler phase%d: caught signal %d.\n", phase, sig);
#endif

	if (phase == 1) {
		tst_brkm(TBROK, cleanup, "A signal was caught before being released.");

	} else {
		/* phase 2 (error if s gets incremented twice) */
		++s;

		if (s > 1) {
			tst_brkm(TBROK, cleanup, "Error occured in signal handler.");
		}

		/* increment the array element for this signal */
		++sig_array[sig];
		sig_caught = TRUE;	/* flag for wait_a_while () */
		--s;
	}

	return;

}				/* end of handler */

/*****************************************************************************
 *  wait_a_while () : wait a while before returning.
 ****************************************************************************/
static void wait_a_while(void)
{
	long btime;

	btime = time(NULL);
	while (time(NULL) - btime < TIMEOUT) {
		if (sig_caught == TRUE)
			break;
	}
}				/* end of wait_a_while */

#ifdef VAX
static int sighold(int signo)
{
	return 0;
}

static int sigrelse(signo)
int signo;
{
	return 0;
}
#endif

int choose_sig(int sig)
{
	switch (sig) {

	case SIGKILL:
	case SIGSTOP:
	case SIGTSTP:
	case SIGCONT:
	case SIGALRM:
#ifdef SIGNOBDM
	case SIGNOBDM:
#endif
#ifdef SIGTTIN
	case SIGTTIN:
#endif
#ifdef SIGTTOU
	case SIGTTOU:
#endif
#ifdef  SIGPTINTR
	case SIGPTINTR:
#endif
#ifdef  SIGSWAP
	case SIGSWAP:
#endif
		return 0;

	}

	return 1;

}

void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	pid = getpid();
}

void cleanup(void)
{
	tst_rmdir();

}
