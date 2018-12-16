/*
* Copyright (c) International Business Machines Corp., 2007
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
* the GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*
***************************************************************************
* File: pidns13.c
* *
* * Description:
* *  The pidns13.c testcase checks container init, for async I/O
* *  triggered by peer namespace process.
* *
* * Test Assertion & Strategy:
* *  Create a pipe in parent namespace.
* *  Create two PID namespace containers(cinit1 and cinit2).
* *  In cinit1, set pipe read end to send SIGUSR1.
* *    for asynchronous I/O.
* *  Let cinit2 to trigger async I/O on pipe write end.
* *  In signal info, check si_code to be POLL_IN and si_fd to be pipe read fd.
* *
* * Usage: <for command-line>
* *  pidns13
* *
* * History:
* *  DATE      NAME                             DESCRIPTION
* *  23/10/08  Gowrishankar M 			Created test scenarion.
* *            <gowrishankar.m@in.ibm.com>
*
******************************************************************************/
#define _GNU_SOURCE 1
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "pidns_helper.h"
#include "test.h"

char *TCID = "pidns13";
int TST_TOTAL = 1;
int pipe_fd[2];

#define CHILD_PID       1
#define PARENT_PID      0

/*
 * child_signal_handler() - dummy function for sigaction()
 */
static void child_signal_handler(int sig, siginfo_t * si, void *unused)
{
	/* sigtimedwait() traps siginfo details, so this wont be called */
	tst_resm(TWARN, "cinit(pid %d): control should have not reached here!",
		 getpid());
}

/*
 * child_fn() - Inside container
 */
int child_fn(void *arg)
{
	struct sigaction sa;
	sigset_t newset;
	siginfo_t info;
	struct timespec timeout;
	pid_t pid, ppid;
	intptr_t cinit_no = (intptr_t)arg;

	/* Set process id and parent pid */
	pid = getpid();
	ppid = getppid();
	if (pid != CHILD_PID || ppid != PARENT_PID) {
		tst_resm(TBROK, "cinit%d: pidns is not created.", cinit_no);
	}

	if (cinit_no == 1) {
		/* in container 1 */
		/* close pipe write descriptor */
		if (close(pipe_fd[1]) == -1) {
			tst_resm(TBROK, "cinit1: close(pipe_fd[1]) failed");
		}

		/* Let cinit1 to get SIGUSR1 on I/O availability */
		if (fcntl(pipe_fd[0], F_SETOWN, pid) == -1) {
			tst_resm(TBROK, "cinit1: fcntl(F_SETOWN) failed");
		}

		if (fcntl(pipe_fd[0], F_SETSIG, SIGUSR1) == -1) {
			tst_resm(TBROK, "cinit1: fcntl(F_SETSIG) failed");
		}

		if (fcntl(pipe_fd[0], F_SETFL,
			  fcntl(pipe_fd[0], F_GETFL) | O_ASYNC) == -1) {
			tst_resm(TBROK, "cinit1: fcntl(F_SETFL) failed");
		}

		/* Set signal handler for SIGUSR1, also mask other signals */
		sa.sa_flags = SA_SIGINFO;
		sigfillset(&sa.sa_mask);
		sa.sa_sigaction = child_signal_handler;
		if (sigaction(SIGUSR1, &sa, NULL) == -1) {
			tst_resm(TBROK, "cinit1: sigaction() failed");
		}

		tst_resm(TINFO, "cinit1: setup handler for async I/O on pipe");

		/* Set timeout for sigtimedwait */
		timeout.tv_sec = 10;
		timeout.tv_nsec = 0;

		/* Set mask to wait for SIGUSR1 signal */
		sigemptyset(&newset);
		sigaddset(&newset, SIGUSR1);

		/* Wait for SIGUSR1 */
		if (sigtimedwait(&newset, &info, &timeout) != SIGUSR1) {
			tst_resm(TBROK, "cinit1: sigtimedwait() failed.");
		}

		/* Recieved SIGUSR1. Check details. */
		if (info.si_fd == pipe_fd[0] && info.si_code == POLL_IN)
			tst_resm(TPASS, "cinit1: si_fd is %d, si_code is %d",
				 info.si_fd, info.si_code);
		else
			tst_resm(TFAIL, "cinit1: si_fd is %d, si_code is %d",
				 info.si_fd, info.si_code);

		/* all done, close the descriptors opened */
		close(pipe_fd[0]);

	} else {
		/* in container 2 */
		/* close pipe read descriptor */
		if (close(pipe_fd[0]) == -1) {
			tst_resm(TBROK, "cinit2: close(pipe_fd[0]) failed");
		}

		/* sleep for few seconds to avoid race with cinit1 */
		sleep(2);

		/* Write some data in pipe to SIGUSR1 cinit1 */
		tst_resm(TINFO, "cinit2: writing some data in pipe");
		if (write(pipe_fd[1], "test\n", 5) == -1) {
			tst_resm(TBROK, "cinit2: write() failed");
		}

		/* all done, close the descriptors opened */
		close(pipe_fd[1]);
	}

	/* cleanup and exit */
	exit(0);
}

static void setup(void)
{
	tst_require_root();
	check_newpid();
}

/***********************************************************************
*   M A I N
***********************************************************************/

int main(int argc, char *argv[])
{
	int status;
	pid_t cpid1, cpid2;

	setup();

	/* create pipe */
	if (pipe(pipe_fd) == -1) {
		tst_resm(TBROK, "parent: pipe creation failed");
	}

	/* Create container 1 */
	cpid1 = ltp_clone_quick(CLONE_NEWPID | SIGCHLD, child_fn, (void*)(intptr_t)1);

	/* Create container 2 */
	cpid2 = ltp_clone_quick(CLONE_NEWPID | SIGCHLD, child_fn, (void*)(intptr_t)2);
	if (cpid1 < 0 || cpid2 < 0) {
		tst_resm(TBROK, "parent: clone() failed.");
	}

	/* Close unwanted descriptors */
	close(pipe_fd[0]);
	close(pipe_fd[1]);

	/* Wait for containers to exit */
	if (waitpid(cpid2, &status, 0) < 0)
		tst_resm(TWARN, "parent: waitpid(cpid2) failed.");

	if (WIFSIGNALED(status) && WTERMSIG(status))
		tst_resm(TWARN, "parent: cinit2 is terminated by signal(%s)",
			 strsignal(WTERMSIG(status)));

	if (waitpid(cpid1, &status, 0) < 0)
		tst_resm(TWARN, "parent: waitpid(cpid1) failed.");

	if (WIFSIGNALED(status) && WTERMSIG(status))
		tst_resm(TWARN, "parent: cinit1 is terminated by signal(%s)",
			 strsignal(WTERMSIG(status)));

	/* Control won't reach below */
	exit(0);

}
