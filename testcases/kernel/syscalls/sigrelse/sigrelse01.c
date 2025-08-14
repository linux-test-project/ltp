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

/* Needed for NPTL */
#define SIGCANCEL 32
#define SIGTIMER 33

void setup(void);
void cleanup(void);
static void parent(void);
static void child(void);
static void timeout(int sig);
static int setup_sigs(void);
static void handler(int sig);
static void wait_a_while(void);
static char *read_pipe(int fd);
static int write_pipe(int fd, char *msg);
static int set_timeout(void);
static void clear_timeout(void);
static void getout(void);
int choose_sig(int sig);

#define TRUE  1
#define FALSE 0

#ifndef DEBUG
#define DEBUG 0
#endif

#define CHILD_EXIT(VAL) ((VAL >> 8) & 0377)	/* exit value of child process */
#define CHILD_SIG(VAL)   (VAL & 0377)	/* signal value of child proc */

#define MAXMESG 512		/* the size of the message string */

#define READY "ready"		/* signal to parent that child is set up */

#define TIMEOUT 30		/* time (sec) used in the alarm calls */

/* child exit values */
#define EXIT_OK    0
#define SIG_CAUGHT 8
#define WRITE_BROK 16
#define HANDLE_ERR 32

int TST_TOTAL = 1;		/* number of test items */

char *TCID = "sigrelse01";	/* test case identifier */
static char mesg[MAXMESG];	/* message buffer for tst_res */
static int pid;			/* process id of child */
static int pipe_fd[2];		/* file descriptors for pipe parent read */
static int pipe_fd2[2];		/* file descriptors for pipe child read */
static int phase;		/* flag for phase1 or phase2 of */
				/* signal handler */
static int sig_caught;		/* flag TRUE if signal caught */
				/* (see wait_a_while ()) */

/* ensure that NUMSIGS is defined. */
#ifndef NUMSIGS
#define NUMSIGS NSIG
#endif

/* array of counters for signals caught by handler() */
static int sig_array[NUMSIGS];

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

		/*
		 * fork off a child process
		 */
		if ((pid = tst_fork()) < 0)
			tst_brkm(TBROK | TERRNO, cleanup, "fork() failed");
		else if (pid > 0)
			parent();
		else
			child();

	}

	cleanup();
	tst_exit();

}				/* end main */

/****************************************************************************
 * parent() : wait for "ready" from child, send signals to child, wait for
 *    child to exit and report what happened.
 ***************************************************************************/
static void parent(void)
{
	int term_stat;		/* child return status */
	int rv;			/* function return value */
	int sig;		/* current signal number */
	char *str;		/* string returned from read_pipe() */
	int *array;		/* pointer to sig_array returned from child */
	int fail = FALSE;	/* flag indicating test item failure */
	char big_mesg[MAXMESG * 6];	/* storage for big failure message */
	int caught_sigs;

	/* wait for "ready" message from child */
	if ((str = read_pipe(pipe_fd[0])) == NULL) {
		/* read_pipe() failed. */
		tst_brkm(TBROK, getout, "%s", mesg);
	}

	if (strcmp(str, READY) != 0) {
		/* child setup did not go well */
		tst_brkm(TBROK, getout, "%s", str);
	}

	/*
	 * send signals to child and see if it holds them
	 */

	for (sig = 1; sig < NUMSIGS; sig++) {
		if (choose_sig(sig)) {
			if (kill(pid, sig) < 0) {
				if (errno == ESRCH) {
					if (kill(pid, SIGTERM) < 0)
						tst_brkm(TBROK | TERRNO, getout,
							 "kill(%d, %d) and kill(%d, SIGTERM) failed",
							 pid, sig, pid);
					else
						tst_brkm(TBROK | TERRNO, getout,
							 "kill(%d, %d) failed, but kill(%d, SIGTERM) worked",
							 pid, sig, pid);
				} else
					tst_brkm(TBROK | TERRNO, getout,
						 "kill(%d, %d) failed", pid,
						 sig);
			}
		}
	}

	if (write_pipe(pipe_fd2[1], READY) < 0) {
		tst_brkm(TBROK | TERRNO, getout,
			 "Unable to tell child to go, write to pipe failed");
	}

	/*
	 * child is now releasing signals, wait and check exit value
	 */
	SAFE_WAIT(getout, &term_stat);

	/* check child's signal exit value */
	if ((sig = CHILD_SIG(term_stat)) != 0)
		/* the child was zapped by a signal */
		tst_brkm(TBROK, cleanup, "Unexpected signal %d killed child",
			 sig);

	/* get child exit value */

	rv = CHILD_EXIT(term_stat);

	switch (rv) {
	case EXIT_OK:
		/* sig_array sent back on pipe, check it out */
		if ((array = (int *)read_pipe(pipe_fd[0])) == NULL) {
			/* read_pipe() failed. */
			tst_resm(TBROK, "%s", mesg);
			break;
		}
#if DEBUG > 1
		for (sig = 1; sig < NUMSIGS; sig++) {
			printf("array[%d] = %d\n", sig, array[sig]);
		}
#endif
		caught_sigs = 0;
		for (sig = 1; sig < NUMSIGS; sig++) {
			if (choose_sig(sig)) {
				if (array[sig] != 1) {
					/* sig was not caught or caught too many times */
					(void)sprintf(mesg,
						      "\tsignal %d caught %d times (expected 1).\n",
						      sig, array[sig]);
					(void)strcat(big_mesg, mesg);
					fail = TRUE;
				} else {
					caught_sigs++;
				}
			}
		}		/* endfor */

		if (fail == TRUE)
			tst_resm(TFAIL, "%s", big_mesg);
		else
			tst_resm(TPASS,
				 "sigrelse() released all %d signals under test.",
				 caught_sigs);
		break;

	case TBROK:
		/* get BROK message from pipe */
		if ((str = read_pipe(pipe_fd[0])) == NULL) {
			/* read_pipe() failed. */
			tst_resm(TBROK, "%s", mesg);
			break;
		}

		/* call tst_res: str contains the message */
		tst_resm(TBROK, "%s", str);
		break;
	case SIG_CAUGHT:
		/* a signal was caught before it was released */
		tst_resm(TBROK, "A signal was caught before being released.");
		break;
	case WRITE_BROK:
		/* the write() call failed in child's write_pipe */
		tst_resm(TBROK, "write() pipe failed for child.");
		break;
	case HANDLE_ERR:
		/* more than one signal tried to be handled at the same time */
		tst_resm(TBROK, "Error occured in signal handler.");
		break;
	default:
		tst_resm(TBROK, "Unexpected exit code %d from child", rv);
		break;
	}

}				/* end of parent */

/****************************************************************************
 * child() : hold signals, notify parent and wait for parent to send signals.
 *   If none were caught (sighold worked), release the signals one at a time
 *   and wait for them to be caught.  Send results back to parent
 *   for processing.
 ***************************************************************************/
static void child(void)
{
	int rv;			/* return value from sighold() and sigrelse() */
	int sig;		/* signal value */
	int exit_val;		/* exit value to send to parent */
	char note[MAXMESG];	/* message buffer for pipe */
	char *str;

	phase = 1;		/* tell handler that we do not want to catch signals */

	/* set note to READY and if an error occurs, overwrite it */
	(void)strcpy(note, READY);

	/* set alarm in case something hangs */
	if (set_timeout() < 0) {
		/* an error occured - put mesg in note and send it back to parent */
		(void)strcpy(note, mesg);

	} else if (setup_sigs() < 0) {
		/* an error occured - put mesg in note and send it back to parent */
		(void)strcpy(note, mesg);

	} else {
		/* all set up to catch signals, now hold them */

		for (sig = 1; sig < NUMSIGS; sig++) {
			if (choose_sig(sig)) {
				if ((rv = sighold(sig)) != 0) {
					/* THEY say sighold ALWAYS returns 0 */
					(void)sprintf(note,
						      "sighold did not return 0. rv:%d",
						      rv);
					break;
				}
			}
		}

	}

	/*
	 * send note to parent (if not READY, parent will BROK) and
	 * wait for parent to send signals.  The timeout clock is set so
	 * that we will not wait forever - if sighold() did its job, we
	 * will not receive the signals.  If sighold() blew it we will
	 * catch a signal and the interrupt handler will exit with a
	 * value of SIG_CAUGHT.
	 */
	if (write_pipe(pipe_fd[1], note) < 0) {
		/*
		 * write_pipe() failed.  Set exit value to WRITE_BROK to let
		 * parent know what happened
		 */
		clear_timeout();
		exit(WRITE_BROK);
	}

	/*
	 * if we get to this point, all signals have been held and the
	 * timer has expired.  Now what we want to do is release each
	 * signal and see if we catch it.  If we catch all signals,
	 * sigrelse passed, else it failed.
	 */

	phase = 2;		/* let handler know we are now expecting signals */

#if DEBUG > 0
	printf("child: PHASE II\n");
#endif

	/* assume success and overwrite exit_val if an error occurs */
	exit_val = EXIT_OK;

#if DEBUG > 0
	printf("child: pid=%d waiting for parent's ready...\n", getpid());
#endif

	/*
	 * wait for parent to tell us that sigals were all sent
	 */

	/* wait for "ready" message from parent */
	if ((str = read_pipe(pipe_fd2[0])) == NULL) {
		/* read_pipe() failed. */
		printf(" child: read_pipe failed\n");
		exit(TBROK);
	}

	if (strcmp(str, READY) != 0) {
		/* parent/pipe problem */
		printf("child: didn't proper ready message\n");
		exit(TBROK);
	}

	for (sig = 1; sig < NUMSIGS; sig++) {
		if (choose_sig(sig)) {

			/* all set up, release and catch a signal */

			sig_caught = FALSE;	/* handler sets it to TRUE when caught */
#if DEBUG > 1
			printf("child: releasing sig %d...\n", sig);
#endif
			if ((rv = sigrelse(sig)) != 0) {
				/* THEY say sigrelse ALWAYS returns 0 */
				(void)sprintf(note,
					      "sigrelse did not return 0. rv:%d",
					      rv);
				exit_val = TBROK;
				break;
			}

			/* give signal handler some time to process signal */
			wait_a_while();
		}

	}			/* endfor */

	/*
	 * If we are error free so far...
	 * check the sig_array array for one occurence of
	 * each of the catchable signals.  If this is true,
	 * then PASS, otherwise FAIL.
	 */

	/* send note to parent and exit */
	if (exit_val == EXIT_OK) {
		if (write(pipe_fd[1], sig_array, sizeof(sig_array)) < 0) {
			tst_resm(TBROK | TERRNO, "write() pipe failed");
			exit(WRITE_BROK);
		}
	} else if (write_pipe(pipe_fd[1], note) < 0) {
		/*
		 * write_pipe() failed.  Set exit value to WRITE_BROK to let
		 * parent know what happened
		 */
		exit(WRITE_BROK);
	}

	exit(exit_val);

}				/* end of child */

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
	for (sig = 1; sig < NUMSIGS; sig++) {
		if (choose_sig(sig)) {
			if (signal(sig, handler) == SIG_ERR) {
				/* set up mesg to send back to parent */
				(void)sprintf(mesg,
					      "signal() failed for signal %d. error:%d %s.",
					      sig, errno, strerror(errno));
				return (-1);
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
		/* exit the child process with a value of -1 */
		exit(SIG_CAUGHT);

	} else {
		/* phase 2 (error if s gets incremented twice) */
		++s;

		if (s > 1) {
			exit(HANDLE_ERR);
		}

		/* increment the array element for this signal */
		++sig_array[sig];
		sig_caught = TRUE;	/* flag for wait_a_while () */
		--s;
	}

	return;

}				/* end of handler */

/*****************************************************************************
 *  read_pipe() : read data from pipe and return in buf.  If an error occurs
 *      put message in mesg and return NULL.  Note: this routine sets a
 *      timeout signal in case the pipe is blocked.
 ****************************************************************************/
static char *read_pipe(int fd)
{
	static char buf[MAXMESG];	/* buffer for pipe read */
	int ret;

#if DEBUG > 0
	printf("read_pipe: pid=%d waiting...\n", getpid());
#endif

	/* set timeout alarm in case the pipe is blocked */
	if (set_timeout() < 0) {
		/* an error occured, message in mesg */
		return NULL;
	}

	ret = -1;
	while (ret == -1) {	/* while empty reads */
		if ((ret = read(fd, buf, MAXMESG)) == 0) {
			(void)sprintf(mesg, "read() pipe failed. error:%d %s.",
				      errno, strerror(errno));

			clear_timeout();
			return NULL;
		}
	}
	clear_timeout();

#if DEBUG > 0
	printf("read_pipe: pid=%d received: %s.\n", getpid(), buf);
#endif
	return (buf);

}				/* end of read_pipe */

/*****************************************************************************
 *  write_pipe(msg) : write msg to pipe.  If it fails, put message in
 *       mesg and return -1, else return 0.
 ****************************************************************************/
static int write_pipe(int fd, char *msg)
{

#if DEBUG > 0
	printf("write_pipe: pid=%d, sending %s.\n", getpid(), msg);
#endif

	if (write(fd, msg, strlen(msg) + 1) < 0) {
		(void)sprintf(mesg, "write() pipe failed. error:%d %s.",
			      errno, strerror(errno));

		return (-1);
	}
	return 0;

}				/* end of write_pipe */

/*****************************************************************************
 *  set_timeout() : set alarm to signal process after the period of time
 *       indicated by TIMEOUT.  If the signal occurs, the routine timeout()
 *       will be executed.  If all goes ok, return 0, else load message
 *       into mesg and return -1.
 ****************************************************************************/
static int set_timeout(void)
{
	if (signal(SIGALRM, timeout) == SIG_ERR) {
		(void)sprintf(mesg,
			      "signal() failed for signal %d. error:%d %s.",
			      SIGALRM, errno, strerror(errno));
		return (-1);
	}

	(void)alarm(TIMEOUT);
	return 0;

}				/* end of set_timeout */

/*****************************************************************************
 *  clear_timeout() : turn off the alarm so that SIGALRM will not get sent.
 ****************************************************************************/
static void clear_timeout(void)
{
	(void)alarm(0);

}				/* end of clear_timeout */

/*****************************************************************************
 *  timeout() : this routine is executed when the SIGALRM signal is
 *      caught.  It does nothing but return - the read() on the pipe
 *      will fail.
 ****************************************************************************/
static void timeout(int sig)
{
#if DEBUG > 0
	printf("timeout: pid=%d sigalrm caught.\n", getpid());
#endif
}

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

static void getout(void)
{
	if (pid > 0 && kill(pid, SIGKILL) < 0)
		tst_resm(TWARN, "kill(%d, SIGKILL) failed", pid);
	cleanup();

}				/* end of getout */

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
	case SIGCANCEL:
	case SIGTIMER:
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

	if (sig < 32)
		return 1;

	return sig >= SIGRTMIN && sig <= SIGRTMAX;
}

void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	/* set up pipe for parent/child communications */
	SAFE_PIPE(cleanup, pipe_fd);

	/*
	 * Cause the read to return 0 once EOF is encountered and the
	 * read to return -1 if pipe is empty.
	 */
	if (fcntl(pipe_fd[0], F_SETFL, O_NONBLOCK) == -1)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "fcntl(Fds[0], F_SETFL, O_NONBLOCK) failed");

	/* set up pipe for parent/child communications */
	SAFE_PIPE(cleanup, pipe_fd2);

	/*
	 * Cause the read to return 0 once EOF is encountered and the
	 * read to return -1 if pipe is empty.
	 */
	if (fcntl(pipe_fd2[0], F_SETFL, O_NONBLOCK) == -1)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "fcntl(Fds[0], F_SETFL, O_NONBLOCK) failed");
}

void cleanup(void)
{
	tst_rmdir();

}
