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
/* $Id: kill02.c,v 1.10 2009/08/28 13:20:15 vapier Exp $ */
/***********************************************************************************

    OS Test -  Silicon Graphics, Inc.

    TEST IDENTIFIER :  kill02  Sending a signal to processes with the same process group ID.

    PARENT DOCUMENT :  kiltds01  Kill System Call.

    AUTHOR          :  Dave Baumgartner

    CO-PILOT        :  Barrie Kletscher

    DATE STARTED    :  12/30/85

    TEST ITEMS

	1. Sending a signal to pid of zero sends to all processes whose process
	   group ID is equal to the process group ID as the sender.

	2. Sending a signal to pid of zero does not send to processes in another process group.

    OUTPUT SPECIFICATIONS

	PASS :
		kiltcs02 1 PASS The signal was sent to all processes in the process group.
		kiltcs02 2 PASS The signal was not sent to selective processes that were not in the process group.

	FAIL :
		kiltcs02 1 FAIL The signal was not sent to all processes in the process group.
		kiltcs02 2 FAIL The signal was sent to a process that was not in the process group.

	BROK :
		kiltcs02 # BROK System call XXX failed. Errno:X, Error message:XXX.
		kiltcs02 # BROK Setting to catch unexpected signal %d failed. Errno: %d, Error message %s.
		kiltcs02 # BROK Setting to ignore signal %d failed. Errno: %d, Error message %s.

	WARN :
		kiltcs02 0 WARN Unexpected signal X was caught.

    SPECIAL PROCEDURAL REQUIREMENTS

	The program must be linked with tst_res.o.

    DETAILED DESCRIPTION

	**Setup**
	Set up unexpected signal handling.
	Set up one pipe for each process to be created with no blocking for read.

	**MAIN**
	If setup fails exit.
	Fork 2 children(1 & 2).
	Wait for set up complete messages from the 1st and 2nd child.
	Send the signal SIGUSR1 with pid equal to zero.
	Sleep a reasonable amount of time so that each child has been swapped in
	to process the signal.
	Now decide the outcome of the test items by reading from each pipe to find
	out if the child was interrupted by the signal and wrote to it.
	Remove the second child.
	Tell the first child it is time to remove it's child B because the decisions have been made.
	Exit.

	**First Child**
	Set to catch SIGUSR1 with an int_rout1.
	Set up to handle the message from the parent to remove child B.
	Fork two children(A & B).
	Wait for set up complete messages from child A & child B.
	Send a set up complete message to the parent.
	Pause until the signal SIGUSR1 comes in from the parent.
	Pause until the parent says it is time to remove the child.
	Exit.

	**Second Child**
	Set to catch SIGUSR1 with an int_rout2.
	Set the process group to be something different than the parents.
	Send a set up complete message to the parent.
	Pause until killed by parent because this child shouldn't receive signal SIGUSR1.

	**Child A**
	Set to catch SIGUSR1 with an int_routA.
	Send a set up complete message to the parent(First Child).
	Pause until the signal SIGUSR1 comes in from the parent.
	Exit.

	**Child B**
	Set to catch SIGUSR1 with an int_routB.
	Set the process group to be something different than the parents(First Child's).
	Send a set up complete message to the parent.
	Pause until killed by parent because this child shouldn't receive signal SIGUSR1.

	**usr1_rout-Used by all children**
	Write to the appropriate pipe that the signal SIGUSR1 was caught.

	**usr2_rout**
	Remove child B.

******************************************************************************/
#include <sys/param.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include "test.h"
#include "usctest.h"

#define CHAR_SET_FAILED	"0"	/*Set up failing status transferred through the pipe.   */
#define CHAR_SET_PASSED	"1"	/*Set up passing status transferred through the pipe.   */
#define SIG_CAUGHT	"2"	/*Indicates that the signal SIGUSR1 was caught.         */
#define SIG_RECEIVED	1	/*Integer value that indicates that the signal SIGUSR1  */
				/*was caught.                                           */
#define SIG_NOT_RECD	0	/*Integer value that indicates that the signal SIGUSR1  */
				/*was caught.                                           */
#define INT_SET_FAILED	0	/*Set up failing status transferred through the pipe.   */
#define INT_SET_PASSED	1	/*Set up passing status transferred through the pipe.   */
#define SLEEP_TIME	10	/*Amount of time the children get to catch the signal   */
#define TRUE	 	40	/*Child exits with this if execution was as     */
				/*expected.                                             */
#define FALSE	 	50	/*Child exits with this if it timed out waiting for the         */
				/*parents signal.                                       */
#define TIMEOUT		60	/*Amount of time given in alarm calls.                  */
#define CHILD_EXIT(VAR) ((VAR >> 8) & 0377)	/*Exit value from the child.               */
#define CHILD_SIG(VAR) (VAR & 0377)	/*Signal value from the termination of child.       */
				/*from the parent.                                      */

int pid1;			/*Return value from 1st fork. Global so that it can be  */
				/*used in interrupt handling routines.                  */
int pid2;			/*Return value from 2nd fork. Global so that it can be  */
				/*used in interrupt handling routines.                  */
int pidA;			/*Return value from 1st fork in child 1. Global so that it      */
				/*can be used in interrupt handling routines.           */
int pidB;			/*Return value from 2nd fork in child 1. Global so that it      */
				/*can be used in interrupt handling routines.           */
int pipe1_fd[2];		/*Pipe file descriptors used for communication          */
				/*between child 1 and the 1st parent.                   */
int pipe2_fd[2];		/*Pipe file descriptors used for communication          */
				/*between child 2 and the 1st parent.                   */
int pipeA_fd[2];		/*Pipe file descriptors used for communication          */
				/*between child A and the 1st parent.                   */
int pipeB_fd[2];		/*Pipe file descriptors used for communication          */
				/*between child B and the 1st parent.                   */
char pipe_buf[10];		/*Pipe buffer.                                          */
char buf_tmp1[2], buf_tmp2[2];	/*Temp hold for info read into pipe_buf.                */
int read1_stat = 0;		/*Number of characters read from pipe 1.                */
int read2_stat = 0;		/*Number of characters read from pipe 2.                */
int readA_stat = 0;		/*Number of characters read from pipe A.                */
int readB_stat = 0;		/*Number of characters read from pipe B.                */
int alarm_flag = FALSE;		/*This flag indicates an alarm time out.                        */
char who_am_i = '0';		/*This indicates which process is which when using      */
				/*interrupt routine usr1_rout.                          */

void notify_timeout();		/*Signal handler that the parent enters if it times out */
				/*waiting for the child to indicate its set up status.  */
void parent_rout();		/*This is the parents routine.                          */
void child1_rout();		/*This is child 1's routine.                            */
void child2_rout();		/*This is child 2's routine.                            */
void childA_rout();		/*This is child A's routine.                            */
void childB_rout();		/*This is child B's routine.                            */
void usr1_rout();		/*This routine is used by all children to indicate that */
				/*they have caught signal SIGUSR1.                      */
void par_kill();		/*This routine is called by the original parent to      */
				/*remove child 2 and to indicate to child 1 to          */
				/*remove its children.                                  */
void chld1_kill();		/*This routine is used by child 1 to remove itself and  */
				/*its children A and B.                                 */

void setup();
void cleanup();

char *TCID = "kill02";		/* Test program identifier.    */
int TST_TOTAL = 2;		/* Total number of test cases. */

int exp_enos[] = { 0 };		/* Array of expected errnos */

#ifdef UCLINUX
static char *argv0;
void childA_rout_uclinux();
void childB_rout_uclinux();
#endif

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

#ifdef UCLINUX
	argv0 = av[0];

	maybe_run_child(&childA_rout_uclinux, "nd", 1, &pipeA_fd[1]);
	maybe_run_child(&childB_rout_uclinux, "nd", 2, &pipeB_fd[1]);
	maybe_run_child(&child1_rout, "ndddddd", 3, &pipe1_fd[1], &pipe2_fd[1],
			&pipeA_fd[0], &pipeA_fd[1], &pipeB_fd[0], &pipeB_fd[1]);
	maybe_run_child(&child2_rout, "nd", 4, &pipe2_fd[1]);
#endif

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		if ((pid1 = FORK_OR_VFORK()) > 0) {
			if ((pid2 = FORK_OR_VFORK()) > 0) {
				(void)parent_rout();
			} else if (pid2 == 0) {
#ifdef UCLINUX
				if (self_exec(argv0, "nd", 4, pipe2_fd[1]) < 0) {
					if (kill(pid1, SIGKILL) == -1
					    && errno != ESRCH) {
						tst_resm(TWARN,
							 "Child process may not have been killed.");
					}
					tst_brkm(TBROK|TERRNO, cleanup, "fork failed");
				}
#else
				(void)child2_rout();
#endif
			} else {
				/*
				 *  The second fork failed kill the first child.
				 */
				if (kill(pid1, SIGKILL) == -1 && errno != ESRCH) {
					tst_resm(TWARN,
						 "Child process may not have been killed.");
				}
				tst_brkm(TBROK|TERRNO, cleanup, "fork failed");
			}

		} else if (pid1 == 0) {
			/*
			 *  This is child 1.
			 */
#ifdef UCLINUX
			if (self_exec
			    (argv0, "ndddddd", 3, pipe1_fd[1], pipe2_fd[1],
			     pipeA_fd[0], pipeA_fd[1], pipeB_fd[0],
			     pipeB_fd[1]) < 0) {
				tst_brkm(TBROK|TERRNO, cleanup, "self_exec() failed");
			}
#else
			(void)child1_rout();
#endif
		} else {
			/*
			 * Fork failed.
			 */
			tst_brkm(TBROK|TERRNO, cleanup, "fork failed");
		}
	}

	cleanup();
	tst_exit();
}				/* END OF MAIN. */

/******************************************************************************
 *  This is the parents routine.  The parent waits for the children 1 and 2 to
 *  get set up. Then sends the signal and checks the outcome.
 *********************************************************************************/
void parent_rout()
{
	/*
	 *  Set to catch the alarm signal SIGALRM.
	 */
	if (signal(SIGALRM, notify_timeout) == SIG_ERR) {
		(void)par_kill();
		tst_brkm(TBROK, NULL,
			 "Could not set to catch the parents time out alarm.");
		tst_exit();
	}

	/*
	 *  Setting to catch the timeout alarm worked now let the children start up.
	 *  Set an alarm which causes a time out on the read pipe loop.
	 *  The children will notify the parent that set up is complete
	 *  and the pass/fail status of set up.
	 */
	(void)alarm(TIMEOUT);

	while ((read(pipe1_fd[0], pipe_buf, 1) != 1) && (alarm_flag == FALSE))
		/*EMPTY*/;
	strncpy(buf_tmp1, pipe_buf, 1);
	(void)alarm(TIMEOUT);

	while ((read(pipe2_fd[0], pipe_buf, 1) != 1) && (alarm_flag == FALSE))
		/*EMPTY*/;
	(void)alarm(0);		/*Reset the alarm clock. */
	strncpy(buf_tmp2, pipe_buf, 1);

	/*
	 *  Check the alarm flag.
	 */
	if (alarm_flag == TRUE) {
		tst_brkm(TBROK, NULL,
			 "The set up of the children failed by timing out.");
		(void)par_kill();
		cleanup();
	}

	/*
	 *  Check to see if either child failed in the set up.
	 */
	if ((strncmp(buf_tmp1, CHAR_SET_FAILED, 1) == 0) ||
	    (strncmp(buf_tmp2, CHAR_SET_FAILED, 1) == 0)) {
		/*
		 * Problems were encountered in the set up of one of the children.
		 * The error message has been displayed by the child.
		 */
		(void)par_kill();
		cleanup();
	}

	/*
	 *  Setup passed, now send SIGUSR1 to process id of zero.
	 */
	TEST(kill(0, SIGUSR1));

	if (TEST_RETURN == -1) {
		tst_brkm(TBROK|TERRNO, NULL, "kill() failed");
		(void)par_kill();
		cleanup();
	}

	/*
	 *  Sleep for a while to allow the children to get a chance to
	 *  catch the signal.
	 */
	(void)sleep(SLEEP_TIME);

	/*
	 *  The signal was sent above and time has run out for child response,
	 *  check the outcomes.
	 */
	read1_stat = read(pipe1_fd[0], pipe_buf, 1);
	if (read1_stat == -1 && errno == EAGAIN)
		read1_stat = 0;
	read2_stat = read(pipe2_fd[0], pipe_buf, 1);
	if (read2_stat == -1 && errno == EAGAIN)
		read2_stat = 0;
	readA_stat = read(pipeA_fd[0], pipe_buf, 1);
	if (readA_stat == -1 && errno == EAGAIN)
		readA_stat = 0;
	readB_stat = read(pipeB_fd[0], pipe_buf, 1);
	if (readB_stat == -1 && errno == EAGAIN)
		readB_stat = 0;

	if (read1_stat == -1 || read2_stat == -1 ||
	    readA_stat == -1 || readB_stat == -1) {
		/*
		 * The read system call failed.
		 */
		tst_brkm(TBROK|TERRNO, NULL, "read() failed");
		(void)par_kill();
		cleanup();
	}

	/*
	 * Check the processes that were supposed to get the signal.
	 */
	if (read1_stat == SIG_RECEIVED) {
		if (readA_stat == SIG_RECEIVED) {
			/*
			 *  Both processes, 1 and A, that were supposed to receive
			 *  the signal did receive the signal.
			 */
			if (STD_FUNCTIONAL_TEST)
				tst_resm(TPASS,
					 "The signal was sent to all processes in the process group.");
			else
				Tst_count++;
		} else {	/*Process A didn't receive the signal. */
			tst_resm(TFAIL,
				 "Process A did not receive the signal.");
		}

	} else {		/*Process 1 didn't receive the signal. */
		tst_resm(TFAIL, "Process 1 did not receive the signal.");
	}

	/*
	 * Check the processes that were not supposed to get the signal.
	 */
	if (read2_stat == SIG_NOT_RECD) {
		if (readB_stat == SIG_NOT_RECD) {
			/*
			 *  Both processes, 2 and B did not receive the signal.
			 */
			if (STD_FUNCTIONAL_TEST)
				tst_resm(TPASS,
					 "The signal was not sent to selective processes that were not in the process group.");
			else
				Tst_count++;
		} else {	/*Process B received the signal. */
			tst_resm(TFAIL, "Process B received the signal.");
		}

	}

	else {			/*Process 2 received the signal. */

		tst_resm(TFAIL, "Process 2 received the signal.");
	}

	(void)par_kill();

	(void)alarm(TIMEOUT);
	while ((read(pipe1_fd[0], pipe_buf, 1) != 1) && (alarm_flag == FALSE))
		strncpy(buf_tmp1, pipe_buf, 1);

}				/*End of parent_rout */

void child1_rout()
{
	who_am_i = '1';

	/*
	 *  Set to catch the SIGUSR1 with int1_rout.
	 */
	if (signal(SIGUSR1, usr1_rout) == SIG_ERR) {
		tst_brkm(TBROK, NULL,
			 "Could not set to catch the childrens signal.");
		(void)write(pipe1_fd[1], CHAR_SET_FAILED, 1);
		exit(0);
	}
	/*
	 *  Create children A & B.
	 */
	if ((pidA = FORK_OR_VFORK()) > 0) {
		/*
		 *  This is the parent(child1), fork again to create child B.
		 */
		if ((pidB = FORK_OR_VFORK()) == 0) {
			/* This is child B. */
#ifdef UCLINUX
			if (self_exec(argv0, "nd", 2, pipeB_fd[1]) < 0) {
				tst_brkm(TBROK|TERRNO, NULL, "self_exec() failed");
				(void)write(pipe1_fd[1], CHAR_SET_FAILED, 1);
				exit(0);
			}
#else
			(void)childB_rout();
#endif
		}

		else if (pidB == -1) {
			/*
			 *  The fork of child B failed kill child A.
			 */
			if (kill(pidA, SIGKILL) == -1)
				tst_resm(TWARN,
					 "Child process may not have been killed.");
			tst_brkm(TBROK|TERRNO, NULL, "fork failed");
			(void)write(pipe2_fd[1], CHAR_SET_FAILED, 1);
			exit(0);
		}
	}

	else if (pidA == 0) {
		/* This is child A. */
#ifdef UCLINUX
		if (self_exec(argv0, "nd", 1, pipeA_fd[1]) < 0) {
			tst_brkm(TBROK|TERRNO, NULL, "self_exec() failed");
			(void)write(pipe1_fd[1], CHAR_SET_FAILED, 1);
			exit(0);
		}
#else
		(void)childA_rout();
#endif

	}

	else if (pidA == -1) {
		/*
		 *  The fork of child A failed.
		 */
		tst_brkm(TBROK|TERRNO, NULL, "fork failed");
		(void)write(pipe1_fd[1], CHAR_SET_FAILED, 1);
		exit(0);
	}

	/*
	 *  Set to catch the SIGUSR2 with chld1_kill.
	 */
	if (signal(SIGUSR2, chld1_kill) == SIG_ERR) {
		tst_brkm(TBROK, NULL,
			 "Could not set to catch the parents signal.");
		(void)write(pipe1_fd[1], CHAR_SET_FAILED, 1);
		(void)chld1_kill();
		exit(0);
	}

	/*
	 *  Set to catch the alarm signal SIGALRM.
	 */
	if (signal(SIGALRM, notify_timeout) == SIG_ERR) {
		tst_brkm(TBROK, NULL,
			 "Could not set to catch the childs time out alarm.");
		(void)write(pipe1_fd[1], CHAR_SET_FAILED, 1);
		(void)chld1_kill();
		exit(0);
	}

	/*
	 *  Setting to catch the signals worked now let the children start up.
	 *  Set an alarm which causes a time out on the pipe read loop.
	 *  The children A & B will notify the parent(child1) that set up is complete
	 *  and the pass/fail status of set up.
	 */
	(void)alarm(TIMEOUT - 40);

	while ((read(pipeA_fd[0], pipe_buf, 1) != 1) && (alarm_flag == FALSE))
		/*EMPTY*/;
	(void)alarm(TIMEOUT - 40);

	while ((read(pipeB_fd[0], pipe_buf, 1) != 1) && (alarm_flag == FALSE))
		/*EMPTY*/;
	(void)alarm(0);		/*Reset the alarm clock. */

	/*
	 *  Check the alarm flag.
	 */
	if (alarm_flag == TRUE) {
		tst_brkm(TBROK, NULL,
			 "The set up of the children failed by timing out.");
		(void)chld1_kill();
		(void)write(pipe1_fd[1], CHAR_SET_FAILED, 1);
		exit(0);
	}

	/*
	 *  Send a set up complete message to the parent.
	 */
	(void)write(pipe1_fd[1], CHAR_SET_PASSED, 1);

	/*
	 *  Pause until the signal SIGUSR1 or SIGUSR2 is sent from the parent.
	 */
	(void)pause();

	/*
	 *  Pause until signal SIGUSR2 is sent from the parent.
	 *  This pause will only be executed if SIGUSR2 has not been received yet.
	 */
	while (1) {
		sleep(1);
	}

}				/*End of child1_rout */

/*******************************************************************************
 *  This is the routine for child 2, which should not receive the parents signal.
 ******************************************************************************/
void child2_rout()
{
	who_am_i = '2';

	/*
	 * Set the process group of this process to be different
	 * than the other processes.
	 */
	(void)setpgrp();

	/*
	 *  Set to catch the SIGUSR1 with usr1_rout.
	 */
	if (signal(SIGUSR1, usr1_rout) == SIG_ERR) {
		tst_brkm(TBROK, cleanup,
			 "Could not set to catch the parents signal.");
		(void)write(pipe2_fd[1], CHAR_SET_FAILED, 1);
		exit(0);
	}

	/* Send a set up complete message to parent. */
	(void)write(pipe2_fd[1], CHAR_SET_PASSED, 1);

	/*
	 *  Pause until killed by the parent or SIGUSR1 is received.
	 */
	(void)pause();
}

/*******************************************************************************
 *  This is the routine for child A, which should receive the parents signal.
 ******************************************************************************/
void childA_rout()
{
	who_am_i = 'A';

	/* Send a set up complete message to parent. */
	write(pipeA_fd[1], CHAR_SET_PASSED, 1);

	/*
	 *  Pause until killed by the parent or SIGUSR1 is received.
	 */
	(void)pause();

	exit(0);
}				/*End of childA_rout */

#ifdef UCLINUX
/*******************************************************************************
 *  This is the routine for child A after self_exec
 ******************************************************************************/
void childA_rout_uclinux()
{
	/* Setup the signal handler again */
	if (signal(SIGUSR1, usr1_rout) == SIG_ERR) {
		tst_brkm(TBROK, NULL,
			 "Could not set to catch the childrens signal.");
		(void)write(pipeA_fd[1], CHAR_SET_FAILED, 1);
		exit(0);
	}

	childA_rout();
}
#endif

/*******************************************************************************
 *  This is the routine for child B, which should not receive the parents signal.
 ******************************************************************************/
void childB_rout()
{
	who_am_i = 'B';

	/*
	 * Set the process group of this process to be different
	 * than the other processes.
	 */
	(void)setpgrp();

	/* Send a set up complete message to parent(child 1). */
	write(pipeB_fd[1], CHAR_SET_PASSED, 1);

	/*
	 *  Pause until killed by the parent(child 1) or SIGUSR1 is received.
	 */
	(void)pause();

	exit(0);
}

#ifdef UCLINUX
/*******************************************************************************
 *  This is the routine for child B after self_exec
 ******************************************************************************/
void childB_rout_uclinux()
{
	/* Setup the signal handler again */
	if (signal(SIGUSR1, usr1_rout) == SIG_ERR) {
		tst_brkm(TBROK, NULL,
			 "Could not set to catch the childrens signal.");
		(void)write(pipeB_fd[1], CHAR_SET_FAILED, 1);
		exit(0);
	}

	childB_rout();
}
#endif

/*******************************************************************************
 *  This routine sets up the interprocess communication pipes, signal handling,
 *  and process group information.
 ******************************************************************************/
void setup()
{
	int errno_buf;		/*indicates the errno if pipe set up fails.             */
	int err_flag = FALSE;	/*Indicates if an error has occurred in pipe set up.    */

	/*
	 *  Set the process group ID to be equal between the parent and children.
	 */
	(void)setpgrp();

	/*
	 *  Set to catch unexpected signals.
	 *  SIGCLD is set to be ignored because we do not wait for termination status.
	 *  SIGUSR1 is set to be ignored because this is the signal we are using for
	 *  the test and we are not concerned with the parent getting it.
	 */

	tst_sig(FORK, DEF_HANDLER, cleanup);

	if (signal(SIGUSR1, SIG_IGN) == SIG_ERR) {
		tst_brkm(TBROK|TFAIL, NULL, "signal(SIGUSR1, SIG_IGN) failed");
		tst_exit();
	}

	if (signal(SIGCLD, SIG_IGN) == SIG_ERR) {
		tst_brkm(TBROK|TERRNO, NULL, "signal(SIGCLD, SIG_IGN) failed");
		tst_exit();
	}

	/* Indicate which errnos are expected */
	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;

	/*
	 *  Set up pipe1, pipe2, pipeA, and pipeB.
	 */
	if ((pipe(pipe1_fd) == -1)
	    || (fcntl(pipe1_fd[0], F_SETFL, O_NDELAY) == -1)) {
		errno_buf = errno;
		err_flag = TRUE;
	}

	if ((pipe(pipe2_fd) == -1)
	    || (fcntl(pipe2_fd[0], F_SETFL, O_NDELAY) == -1)) {
		errno_buf = errno;
		err_flag = TRUE;
	}

	if ((pipe(pipeA_fd) == -1)
	    || (fcntl(pipeA_fd[0], F_SETFL, O_NDELAY) == -1)) {
		errno_buf = errno;
		err_flag = TRUE;
	}

	if ((pipe(pipeB_fd) == -1)
	    || (fcntl(pipeB_fd[0], F_SETFL, O_NDELAY) == -1)) {
		errno_buf = errno;
		err_flag = TRUE;
	}

	/*
	 *  Check for errors.
	 */
	if (err_flag == TRUE) {
		tst_brkm(TBROK|TERRNO, NULL, "pipe() failed");
		tst_exit();
	}
	return;

}

/***********************************************************
 *  This routine indicates that the process caught SIGUSR1.
 **********************************************************/
void usr1_rout()
{
	switch (who_am_i) {
	case '1':
		if (write(pipe1_fd[1], SIG_CAUGHT, 1) == -1)
			tst_resm(TWARN,
				 "Writing signal catching status failed in child 1.");
		break;
	case '2':
		if (write(pipe2_fd[1], SIG_CAUGHT, 1) == -1)
			tst_resm(TWARN,
				 "Writing signal catching status failed in child 2.");
		break;
	case 'A':
		if (write(pipeA_fd[1], SIG_CAUGHT, 1) == -1)
			tst_resm(TWARN,
				 "Writing signal catching status failed in child A.");
		break;
	case 'B':
		if (write(pipeB_fd[1], SIG_CAUGHT, 1) == -1)
			tst_resm(TWARN,
				 "Writing signal catching status failed in child B.");
		break;
	default:
		tst_resm(TWARN,
			"Unexpected value %d for who_am_i in usr1_rout()",
			who_am_i);
		break;
	}

}				/*End of usr1_rout */

/***********************************************************
 *  This routine handles the timeout alarm in the parent,
 *  which occurs when the child fails to notify the parent
 *  the status of set up.
 **********************************************************/
void notify_timeout()
{
	alarm_flag = TRUE;

}				/*End of notify_timeout */

/***********************************************************
 *  This routine handles the procedure for removing the
 *  children forked off during this test.
 **********************************************************/
void par_kill()
{
	int status;

	/*
	 *  Indicate to child1 that it can remove it's children and itself now.
	 */
	if (kill(pid1, SIGUSR2) == -1 && errno != ESRCH) {
		tst_resm(TWARN|TERRNO, "kill() failed");
		tst_resm(TWARN, "Child 1 and it's children may still be alive.");
	}

	/*
	 *  Remove child 2.
	 */
	if (kill(pid2, SIGKILL) == -1 && errno != ESRCH)
		tst_resm(TWARN, "Child2 may still be alive.");

	wait(&status);
	return;

}				/*End of par_kill */

/*********************************************************************
 *  This routine is executed by child 1 when the parent tells it to
 *  remove it's children and itself.
 ********************************************************************/
void chld1_kill()
{
	/*
	 *  Remove children A & B.
	 */
	if (kill(pidA, SIGKILL) == -1 && errno != ESRCH)
		tst_resm(TWARN|TERRNO, "kill(%d) failed; child 1's(A) child may still be alive", pidA);

	(void)write(pipe1_fd[1], CHAR_SET_PASSED, 1);

	if (kill(pidB, SIGKILL) == -1 && errno != ESRCH)
		tst_resm(TWARN|TERRNO, "kill(%d) failed; child 1's(B) child may still be alive", pidB);

	exit(0);

}				/*End of chld1_kill */

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

}
