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
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
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
/* $Id: sighold02.c,v 1.11 2009/08/28 13:53:04 vapier Exp $ */
/*****************************************************************************
 * OS Test - Silicon Graphics, Inc.  Eagan, Minnesota
 *
 * TEST IDENTIFIER : sighold02 Holding all signals.
 *
 * PARENT DOCUMENT : sghtds01  sighold system call (CRAY X-MP and CRAY-1 only)
 *
 * AUTHOR          : Bob Clark
 *
 * CO-PILOT        : Barrie Kletscher
 *
 * DATE STARTED    : 9/26/86
 *
 * TEST ITEMS
 *
 *	1. sighold action to turn off the receipt of all signals was done
 *	   without error.
 *	2. After signals were held, and sent, no signals were trapped.
 *
 * SPECIAL PROCEDURAL REQUIRMENTS
 *
 *	The program must be linked with tst_res.o and parse_opts.o.
 *
 * DETAILED DESCRIPTION
 *
 *	set up pipe for parent/child communications
 *	fork off a child process
 *
 *	PARENT:
 *		set up for unexpected signals
 *		wait for child to send ready message over pipe
 *		issue a result for sighold
 *		send all catchable signals to child
 *	        write to pipe, tell child all signals sent
 *		read pipe to get result from child
 *		wait for child to terminate
 *
 *	CHILD:
 *		set up to catch all signals
 *		hold signals with sighold()
 *		send parent sighold results via pipe
 *		wait for signals to arrive while reading pipe
 *		write to pipe telling parent which signals were received if any.
 *
 ***************************************************************************/

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "test.h"
#include "usctest.h"

/* Needed for NPTL */
#define SIGCANCEL 32
#define SIGTIMER 33

#ifdef _CRAYT3E
#define CRAYT3E 1
#else
#define CRAYT3E 0
#endif

#ifdef sgi
#define SGI 1
#else
#define SGI 0
#endif

#ifdef __linux__
/* glibc2.2 definition needs -D_XOPEN_SOURCE, which breaks other things. */
extern int sighold(int __sig);
#endif

/* ensure NUMSIGS is defined */
#ifndef NUMSIGS
#define NUMSIGS NSIG
#endif

#define CHILD_EXIT(VAL) ((VAL >> 8) & 0377)	/* exit value of child process */

#define MAXMESG 150		/* the size of the message string */

#define TIMEOUT 2		/* time used in the alarm calls as backup */

char *TCID = "sighold02";	/* Test program identifier.    */
int TST_TOTAL = 2;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

char signals_received[MAXMESG];
int pid;			/* process id of child */
int Fds1[2];			/* file descriptors for pipe - child 2 parent */
int Fds2[2];			/* file descriptors for pipe - parent 2 child */

#define  PARENTSREADFD    Fds1[0]
#define  CHILDSWRITEFD    Fds1[1]

#define  CHILDSREADFD     Fds2[0]
#define  PARENTSWRITEFD   Fds2[1]

struct pipe_packet {
	int result;
	char mesg[MAXMESG];
	struct tblock rtimes;
} p_p;

void do_child();
void setup();
void cleanup();
static void getout();
static void timeout();
static int read_pipe(int fd);
static int write_pipe(int fd);
static int setup_sigs(char *mesg);
static void handle_sigs();
static int set_timeout(char *mesg);
static void clear_timeout();

int Timeout = 0;

/***********************************************************************
 * MAIN
 ***********************************************************************/
int main(int ac, char **av)
{
	int term_stat;		/* child return status */
	int sig;		/* current signal */
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

    /***************************************************************
     * parse standard options, and exit if there is an error
     ***************************************************************/
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}
#ifdef UCLINUX
	maybe_run_child(&do_child, "dd", &CHILDSWRITEFD, &CHILDSREADFD);
#endif

    /***************************************************************
     * perform global setup for test
     ***************************************************************/
	setup();

    /***************************************************************
     * check looping state if -c option given
     ***************************************************************/
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		signals_received[0] = '\0';

		/*
		 * fork off a child process
		 */
		if ((pid = FORK_OR_VFORK()) < 0) {
			tst_brkm(TBROK|TERRNO, cleanup, "fork() failed");

		} else if (pid > 0) {

			/* PARENT PROCESS - first set up for unexpected signals */
			tst_sig(FORK, DEF_HANDLER, getout);

			/* wait for "ready" message from child */
			if (read_pipe(PARENTSREADFD) != 0) {
				/* read_pipe() failed. */
				tst_brkm(TBROK, getout, "%s", p_p.mesg);
			}

			if (STD_TIMING_ON) {
				/*
				 * Insure a running total for multiple loop iterations
				 */
				tblock.tb_total += p_p.rtimes.tb_total;
				if (p_p.rtimes.tb_min < tblock.tb_min)
					tblock.tb_min = p_p.rtimes.tb_min;
				if (p_p.rtimes.tb_max > tblock.tb_max)
					tblock.tb_max = p_p.rtimes.tb_max;
				tblock.tb_count += p_p.rtimes.tb_count;
			}

			/* check for ready message */
			if (p_p.result != TPASS) {
				/* child setup did not go well */
				tst_brkm(p_p.result, getout, "%s", p_p.mesg);
			} else if (STD_FUNCTIONAL_TEST) {
				tst_resm(p_p.result, "%s", p_p.mesg);
			} else {	/* no pass results being issued */
				Tst_count++;
			}

			/*
			 * send signals to child and see if it holds them
			 */

			for (sig = 1; sig < NUMSIGS; sig++) {
				if ((sig == 41) && !CRAYT3E && !SGI) {
					sig = 42;	/* skip over SIGPEFAILURE for non-CRAYT3E systems */
				}
				if ((sig != SIGCLD) && (sig != SIGKILL) &&
				    (sig != SIGALRM) && (sig != SIGSTOP)
				    && (sig != SIGCANCEL)
				    && (sig != SIGTIMER)
#ifdef SIGRECOVERY
				    && (sig != SIGRECOVERY)
#endif
#ifdef SIGRESTART
				    && (sig != SIGRESTART)
#endif
#ifdef SIGNOBDM
				    && (sig != SIGNOBDM)
#endif
#ifdef SIGPTINTR
				    && (sig != SIGPTINTR)
#endif
				    ) {
					if (kill(pid, sig) < 0) {
						tst_brkm(TBROK|TERRNO, NULL, "kill(%d, %d) failed", pid, sig);
						getout();
					}
				}
			}

			/*
			 * Tell child that all signals were sent.
			 */
			p_p.result = TPASS;
			strcpy(p_p.mesg, "All signals were sent");
			write_pipe(PARENTSWRITEFD);

			/*
			 * Get childs reply about received signals.
			 */

			if (read_pipe(PARENTSREADFD) < 0) {
				tst_brkm(TBROK, getout, "%s", p_p.mesg);
			}

			if (STD_FUNCTIONAL_TEST)
				tst_resm(p_p.result, "%s", p_p.mesg);

			else if (p_p.result != TPASS)
				tst_resm(p_p.result, "%s", p_p.mesg);

			else
				Tst_count++;

			/*
			 * wait for child
			 */
			if (wait(&term_stat) < 0) {
				tst_brkm(TBROK, getout, "wait() failed");
			}

		} else {

			/*
			 * CHILD PROCESS - set up to catch signals.
			 */

#ifdef UCLINUX
			if (self_exec(av[0], "dd", CHILDSWRITEFD, CHILDSREADFD)
			    < 0) {
				tst_brkm(TBROK|TERRNO, cleanup, "self_exec() failed");
			}
#else
			do_child();
#endif
		}
	}
	cleanup();

	return 0;
}

/*****************************************************************************
 *  do_child()
 ****************************************************************************/

void do_child()
{
	int rv;			/* function return value */
	int sig;		/* current signal */
	int cnt;

	p_p.result = TPASS;

	/* set up signal handlers for the signals */
	if (setup_sigs(p_p.mesg) < 0) {
		p_p.result = TBROK;

	} else {
		/* all set up to catch signals, now hold them */

		for (cnt = 0, sig = 1; sig < NUMSIGS; sig++) {
			if ((sig == 41) && !CRAYT3E && !SGI) {
				sig = 42;	/* skip over SIGPEFAILURE for non-CRAYT3E systems */
			}
			if ((sig != SIGCLD) && (sig != SIGKILL) &&
			    (sig != SIGALRM) && (sig != SIGSTOP)
#ifdef SIGNOBDM
			    && (sig != SIGNOBDM)
#endif
			    && (sig != SIGCANCEL) && (sig != SIGTIMER)
			    ) {

				cnt++;
				TEST(sighold(sig));
				rv = TEST_RETURN;
				if (rv != 0) {
					/* THEY say sighold ALWAYS returns 0 */
					p_p.result = TFAIL;
					(void)sprintf(p_p.mesg,
						      "sighold(%d) failed, rv:%d, errno:%d",
						      sig, rv, errno);
					break;
				}
			}
		}
		if (STD_TIMING_ON) {
			p_p.rtimes = tblock;
		}
		if (p_p.result == TPASS) {
			sprintf(p_p.mesg,
				"Sighold called without error for %d of %d signals",
				cnt, NUMSIGS - 1);
		}
	}

	/*
	 * write to parent (if not READY, parent will BROK) and
	 * wait for parent to send signals.  The timeout clock is set so
	 * that we will not wait forever - if sighold() did its job, we
	 * will not receive the signals.  If sighold() blew it we will
	 * catch a signal and the interrupt handler will exit(1).
	 */
#if debug
	printf("child: %d writing to parent fd:%d\n", getpid(), CHILDSWRITEFD);
#endif
	if (write_pipe(CHILDSWRITEFD) < 0 || p_p.result != TPASS) {
		exit(2);
	}

	/*
	 * Read pipe from parent, that will tell us that all signals were sent
	 */
	if (read_pipe(CHILDSREADFD) != 0) {
		p_p.result = TBROK;
		strcpy(p_p.mesg, "read() pipe failed");
	} else if (signals_received[0] == '\0') {
		p_p.result = TPASS;
		strcpy(p_p.mesg,
		       "No signals trapped after being sent by parent");
	} else {
		p_p.result = TFAIL;
		sprintf(p_p.mesg, "signals received: %s", signals_received);
	}

	if (write_pipe(CHILDSWRITEFD) < 0) {
		exit(2);
	}

	/* exit back to parent */
	if (p_p.result == TPASS)
		exit(0);
	else
		exit(1);
}

/*****************************************************************************
 *  read_pipe() : read data from pipe and return in buf.  If an error occurs
 *      put message in mesg and return NULL.  Note: this routine sets a
 *      timeout signal in case the pipe is blocked.
 ****************************************************************************/

int read_pipe(fd)
int fd;
{
	int ret = -1;

#ifdef debug
	printf("read_pipe: %d entering, fd = %d...\n", getpid(), fd);
#endif

	/* set timeout alarm in case the pipe is blocked */
	if (set_timeout(p_p.mesg) < 0) {
		/* an error occured, message in mesg */
		return -1;
	}

	ret = read(fd, (char *)&p_p, sizeof(struct pipe_packet));

#ifdef debug
	printf("read_pipe: %d read() completed ret=%d\n", getpid(), ret);
#endif

	clear_timeout();

	if (Timeout) {
		(void)sprintf(p_p.mesg,
			      "read() pipe failed -timed out after %d seconds.",
			      TIMEOUT);
		return -1;
	}
#ifdef debug
	printf("read_pipe: received %s.\n", p_p.mesg);
#endif

	return 0;
}

/*****************************************************************************
 *  write_pipe(msg) : write p_p to pipe.  If it fails, put message in
 *         mesg and return -1, else return 0.
 ****************************************************************************/

static int write_pipe(fd)
int fd;
{
#ifdef debug
	printf("write_pipe: sending result:%d, mesg:%s.\n", p_p.result,
	       p_p.mesg);
#endif
	if (write(fd, (char *)&p_p, sizeof(struct pipe_packet)) < 0) {
		if (pid)
			tst_brkm(TBROK|TERRNO, getout, "write() pipe failed");
		return -1;
	}
#ifdef debug
	printf("write_pipe: %d complete successfully, fd:%d.\n", getpid(), fd);
#endif
	return 0;
}

/*****************************************************************************
 *  set_timeout() : set alarm to signal process after the period of time
 *       indicated by TIMEOUT.  If the signal occurs, the routine timeout()
 *       will be executed.  If all goes ok, return 0, else load message
 *       into mesg and return -1.
 ****************************************************************************/

static int set_timeout(char *mesg)
{
	if (signal(SIGALRM, timeout) == SIG_ERR) {
		(void)sprintf(mesg,
			      "signal() failed for signal %d. error:%d %s.",
			      SIGALRM, errno, strerror(errno));
		return (-1);
	}
#if debug
	printf("set_timeout()...\n");
#endif

	Timeout = 0;
	(void)alarm(TIMEOUT);
	return 0;
}

/*****************************************************************************
 *  clear_timeout() : turn off the alarm so that SIGALRM will not get sent.
 ****************************************************************************/

static void clear_timeout()
{
	(void)alarm(0);
	Timeout = 0;
}

/*****************************************************************************
 *  timeout() : this routine is executed when the SIGALRM signal is
 *      caught.  It does nothing but return - if executed during a read()
 *      system call, a -1 will be returned by the read().
 ****************************************************************************/

static void timeout()
{
#ifdef debug
	printf("timeout: sigalrm caught.\n");
#endif
	Timeout = 1;

}

/*****************************************************************************
 *  setup_sigs() : set child up to catch all signals.  If there is
 *       trouble, write message in mesg and return -1, else return 0.
 ****************************************************************************/

static int setup_sigs(char *mesg)
{
	int sig;

	/* set up signal handler routine */
	for (sig = 1; sig < NUMSIGS; sig++) {
		if ((sig != SIGCLD) && (sig != SIGKILL) &&
		    (sig != SIGALRM) && (sig != SIGSTOP)
#ifdef SIGRESTART
		    && (sig != SIGRESTART)
#endif
#ifdef SIGRECOVERY
		    && (sig != SIGRECOVERY)
#endif
#ifdef SIGSWAP
		    && (sig != SIGSWAP)
#endif
		    && (sig != SIGCANCEL) && (sig != SIGTIMER)
		    ) {

			if (signal(sig, handle_sigs) == SIG_ERR) {
				/* set up mesg to send back to parent */
				(void)sprintf(mesg,
					      "signal() failed for signal %d. error:%d %s.",
					      sig, errno, strerror(errno));
				return (-1);
			}
		}
	}
	return 0;
}

/*****************************************************************************
 *  handle_sigs() : interrupt handler for all signals.  This will be run
 *      if the child process catches a signal (resulting in
 *      a test item FAIL if tst_res has not yet been called).  The parent
 *      detects this situation by a child exit value of 1.
 ****************************************************************************/

static void handle_sigs(sig)
int sig;			/* the signal causing the execution of this handler */
{
	char string[10];

#ifdef debug
	printf("child: handle_sigs: caught signal %d.\n", sig);
#endif

	sprintf(string, " %d", sig);
	strcat(signals_received, string);

	return;
}

/*****************************************************************************
 *  getout() : attempt to kill child process and call cleanup().
 ****************************************************************************/

static void getout()
{
	if (kill(pid, SIGKILL) < 0)
		tst_resm(TWARN|TERRNO, "kill(%d) failed", pid);
	cleanup();
}

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void setup()
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* set up pipe for child sending to parent communications */
	if (pipe(Fds1) < 0)
		tst_brkm(TBROK|TERRNO, cleanup, "pipe() failed");

	/* set up pipe for parent sending to child communications */
	if (pipe(Fds2) < 0)
		tst_brkm(TBROK|TERRNO, cleanup, "pipe() failed");

#if debug
	printf("child 2 parent Fds1[0] = %d, Fds1[1] = %d\n", Fds1[0], Fds1[1]);
	printf("parent 2 child Fds2[0] = %d, Fds2[1] = %d\n", Fds2[0], Fds2[1]);
#endif

	/* Pause if that option was specified */
	TEST_PAUSE;

}				/* End setup() */

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *              completion or premature exit.
 ***************************************************************/
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();

}				/* End cleanup() */
