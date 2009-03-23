/* IBM Corporation */
/* 01/02/2003	Port to LTP	avenkat@us.ibm.com */
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

			   /*kill1.c */
/*======================================================================
>KEYS:  < kill(), wait()
>WHAT:  < Check that when a child is killed by its parent, it returns
        < the correct values to the waiting parent--default behaviour
	< assumed by child
>HOW:   < For each signal: send that signal to a child, check that the
	< child returns the correct value to the waiting parent.
>BUGS:  <
>REQUIREMENT(S):  Need to set ulimit to multiples of 1024.
======================================================================*/
#define _GNU_SOURCE 1

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/*****  LTP Port        *****/
#include <errno.h>
#include "test.h"
#include "usctest.h"
#define ITER    3
#define FAILED 0
#define PASSED 1

char *TCID = "kill11";

int local_flag = PASSED;
int block_number;
FILE *temp;
int TST_TOTAL = 1;
extern int Tst_count;
static int sig;

int anyfail();
int blenter();
void setup();
void terror();
void fail_exit();
int forkfail();
void do_child();

/*****  **      **      *****/

//char progname[] = "kill1()";

/*--------------------------------------------------------------------*/
int main(int argc, char **argv)
{
/***** BEGINNING OF MAIN. *****/
	int core;
	int pid, npid;
	int nsig, exno, nexno, status;
	/*SIGIOT is 6, but since linux doesn't have SIGEMT, just using
	   SIGIOT for place filling */
	int signum[15];
	int j;
	int ret_val = 0;
#ifdef UCLINUX
	char *msg;
#endif
	signum[1] = SIGHUP;
	signum[2] = SIGINT;
	signum[3] = SIGQUIT;
	signum[4] = SIGILL;
	signum[5] = SIGTRAP;
	signum[6] = SIGABRT;
	signum[7] = SIGIOT;
	signum[8] = SIGFPE;
	signum[9] = SIGKILL;
	signum[10] = SIGBUS;
	signum[11] = SIGSEGV;
	signum[12] = SIGSYS;
	signum[13] = SIGPIPE;
	signum[14] = SIGALRM;

#ifdef UCLINUX
	/* parse standard options */
	if ((msg =
	     parse_opts(argc, argv, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	maybe_run_child(&do_child, "dd", &temp, &sig);
#endif

	setup();
	// tempdir();           /* move to new directory */
/*--------------------------------------------------------------------*/
	blenter();		/*<<<<<ENTER DATA HERE<<<<<<<< */

	exno = 1;
	unlink("core");

	for (j = 1; j < sizeof(signum) / sizeof(*signum); j++) {
		sig = signum[j];
		if (sig != SIGKILL)
#ifndef BCS
			if (sig != SIGSTOP)
#endif
				if (sigset(sig, SIG_DFL) == SIG_ERR) {
					fprintf(temp, "\tsigset(%d,,) fails\n",
						sig);
					local_flag = FAILED;
					fail_exit();
				}
		fflush(temp);
		pid = FORK_OR_VFORK();

		if (pid < 0) {
			forkfail();
		}

		/*
		 * Child process sleeps for up to 3 minutes giving the
		 * parent process a chance to kill it.
		 */
		if (pid == 0) {
#ifdef UCLINUX
			if (self_exec(argv[0], "dd", temp, sig) < 0) {
				tst_resm(TBROK, "self_exec FAILED - "
					 "terminating test.");
				tst_exit();
				return 0;
			}
#else
			do_child();
#endif
		}

		/*
		 * Parent process sends signal to child.
		 */

		//fprintf(temp, "Testing signal %d\n", sig); 12/12/02
		kill(pid, sig);
		npid = wait(&status);

		if (npid != pid) {
			fprintf(temp, "wait error: unexpected pid returned\n");
			ret_val = 1;
		}
		/* 12/20/02.
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

		//printf("nsig=%x, core=%x, status=%x\n", nsig,core, status); 12/12/2002

		/* to check if the core dump bit has been set, bit # 7 */
	/*****	LTP Port	*****/
		/*  12/12/02: avenkat@us.ibm.com
		 *  SIGILL when is not caught or not ignored it causes
		 *  a core dump and program termination.  So moved the condition to
		 *  else part of the program.
		 *  SIGQUIT like SIGABRT normally causes a program to quit and
		 *  and dumps core.  So moved the condition to else part of the
		 *  program.
		 */
	/*****	**	**	*****/
		if (core) {
			if ((sig == 1) || (sig == 2)
			    /*|| (sig == 3) || */
			    /*(sig == 4) */
			    || (sig == 9) ||
			    (sig == 13) || (sig == 14) || (sig == 15)) {
				fprintf(temp,
					"signal error: core dump bit set for exception number %d\n",
					sig);
				ret_val = 1;
			}
		} else {
			if ((sig == 3) || (sig == 4) || (sig == 5) || (sig == 6)
			    || (sig == 7) || (sig == 8) || (sig == 10)
			    || (sig == 11) || (sig == 12)) {
				fprintf(temp,
					"signal error: core dump bit not set for exception number %d\n",
					sig);
				ret_val = 1;
			}
		}

		/* nsig is the signal number returned by wait */

		if (nsig != sig) {
			fprintf(temp,
				"wait error: unexpected signal %d returned, expected %d\n",
				nsig, sig);
			ret_val = 1;
		}

		/* nexno is the exit number returned by wait  */

		if (nexno != 0) {
			fprintf(temp,
				"signal error: unexpected exit number %d returned, expected 0\n",
				nexno);
			ret_val = 1;
		}
	}
	unlink("core");
	fflush(temp);
	if (ret_val)
		local_flag = FAILED;
	unlink("core");
	tst_rmdir();
/*--------------------------------------------------------------------*/
	anyfail();
	return 0;
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
	register int i;
	int exno = 1;

	for (i = 0; i < 180; i++)
		sleep(1);
	fprintf(temp, "\tChild missed sig %d\n", sig);
	fflush(temp);
	_exit(exno);
}

void setup()
{
	temp = stderr;
	tst_tmpdir();
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
