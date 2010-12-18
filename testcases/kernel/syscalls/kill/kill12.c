/* IBM Corporation */
/* 01/02/2003	Port to LTP avenkat@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
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

			   /*kill2.c */
/*======================================================================
>KEYS:  < kill(), wait(), signal()
>WHAT:  < Check that when a child is killed by its parent, it returns the
   	< correct values to the waiting parent--the child sets signal to
   	< ignore the kill
>HOW:   < For each signal: Send that signal to a child that has elected
	< to catch the signal, check that the correct status was returned
	< to the waiting parent.
	< NOTE: Signal 9 (kill) is not catchable, and must be dealt with
	< separately.
>BUGS:  < None known
======================================================================*/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

//char progname[] = "kill2()";
/*****  LTP Port        *****/
#include "test.h"
#include "usctest.h"
#define ITER    3
#define FAILED 0
#define PASSED 1

char *TCID = "kill12";

int local_flag = PASSED;
int block_number;
FILE *temp;
int TST_TOTAL = 1;
static int sig;

int anyfail();
int blenter();
int instress();
void setup();
void terror();
void fail_exit();
void ok_exit();
int forkfail();
void do_child();

/*****  **      **      *****/

int chflag;

/*--------------------------------------------------------------------*/
int main(int argc, char **argv)
{
/***** BEGINNING OF MAIN. *****/
	int pid, npid;
	int nsig, exno, nexno, status;
	int ret_val = 0;
	int core;
	void chsig();

#ifdef UCLINUX
	char *msg;

	/* parse standard options */
	if ((msg =
	     parse_opts(argc, argv, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	maybe_run_child(&do_child, "dd", &temp, &sig);
#endif

	setup();
	//tempdir();            /* move to new directory */ 12/20/2003
	blenter();

	exno = 1;

	if (sigset(SIGCLD, chsig) == SIG_ERR) {
		fprintf(temp, "\tsigset failed, errno = %d\n", errno);
		fail_exit();
	}

	for (sig = 1; sig < 14; sig++) {
		fflush(temp);
		chflag = 0;

		pid = FORK_OR_VFORK();
		if (pid < 0) {
			forkfail();
		}

		if (pid == 0) {
#ifdef UCLINUX
			if (self_exec(argv[0], "dd", temp, sig) < 0) {
				tst_resm(TBROK, "self_exec FAILED - "
					 "terminating test.");
				tst_exit();
				tst_exit();
			}
#else
			do_child();
#endif
		} else {
			//fprintf(temp, "Testing signal %d\n", sig);

			while (!chflag)	/* wait for child */
				sleep(1);

			kill(pid, sig);	/* child should ignroe this sig */
			kill(pid, SIGCLD);	/* child should exit */

#ifdef BCS
			while ((npid = wait(&status)) != pid
			       || (npid == -1 && errno == EINTR)) ;
			if (npid != pid) {
				fprintf(temp,
					"wait error: wait returned wrong pid\n");
				ret_val = 1;
			}
#else
			while ((npid = waitpid(pid, &status, 0)) != -1
			       || errno == EINTR) ;
#endif

			/*
			   nsig = status & 0177;
			   core = status & 0200;
			   nexno = (status & 0xff00) >> 8;
			 */
			/*****  LTP Port        *****/
			nsig = WTERMSIG(status);
#ifdef WCOREDUMP
			core = WCOREDUMP(status);
#endif
			nexno = WIFEXITED(status);
			/*****  **      **      *****/

			/* nsig is the signal number returned by wait
			   it should be 0, except when sig = 9          */

			if ((sig == 9) && (nsig != sig)) {
				fprintf(temp, "wait error: unexpected signal"
					" returned when the signal sent was 9"
					" The status of the process is %d \n",
					status);
				ret_val = 1;
			}
			if ((sig != 9) && (nsig != 0)) {
				fprintf(temp, "wait error: unexpected signal "
					"returned, the status of the process is "
					"%d  \n", status);
				ret_val = 1;
			}

			/* nexno is the exit number returned by wait
			   it should be 1, except when sig = 9          */

			if (sig == 9)
				if (nexno != 0) {
					fprintf(temp, "signal error: unexpected"
						" exit number returned when"
						" signal sent was 9, the status"
						" of the process is %d \n",
						status);
					ret_val = 1;
				} else;
			else if (nexno != 1) {
				fprintf(temp, "signal error: unexpected exit "
					"number returned,the status of the"
					" process is %d\n", status);
				ret_val = 1;
			}
		}
	}
	if (ret_val)
		local_flag = FAILED;

/*--------------------------------------------------------------------*/
	anyfail();
	tst_exit();
}					/******** END OF MAIN. ********/

/*--------------------------------------------------------------------*/

void chsig()
{
	chflag++;
}

/****** LTP Port        *****/
int anyfail()
{
	(local_flag == FAILED) ? tst_resm(TFAIL,
					  "Test failed") : tst_resm(TPASS,
								    "Test passed");
	tst_exit();
	return 0;
}

void do_child()
{
	int exno = 1;

#ifdef UCLINUX
	if (sigset(SIGCLD, chsig) == SIG_ERR) {
		fprintf(temp, "\tsigset failed, errno = %d\n", errno);
		fail_exit();
	}
#endif

	sigset(sig, SIG_IGN);	/* set to ignore signal */
	kill(getppid(), SIGCLD);	/* tell parent we are ready */
	while (!chflag)
		sleep(1);	/* wait for parent */

	exit(exno);
}

void setup()
{
	temp = stderr;
}

int blenter()
{
	//tst_resm(TINFO, "Enter block %d", block_number);
	local_flag = PASSED;
	return 0;
}

void terror(char *message)
{
	tst_resm(TBROK, "Reason: %s:%s", message, strerror(errno));
}

void fail_exit()
{
	local_flag = FAILED;
	anyfail();

}

int forkfail()
{
	tst_resm(TBROK, "FORK FAILED - terminating test.");
	tst_exit();
	return 0;
}

/****** ** **   *******/