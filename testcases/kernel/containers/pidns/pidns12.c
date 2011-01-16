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
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
***************************************************************************
* File: pidns12.c
* *
* * Description:
* *  The pidns12.c testcase verifies that siginfo->si_pid is set to 0
* *  if sender (parent process) is not in receiver's namespace.
* *
* * Test Assertion & Strategy:
* *  Create a PID namespace container.
* *  Initialise signal handler for SIGUSR1 in container.
* *  Let parent send SIGUSR1 to container.
* *  Check if sender pid is set to 0 from signal info.
* *
* * Usage: <for command-line>
* *  pidns12
* *
* * History:
* *  DATE      NAME                             DESCRIPTION
* *  13/11/08  Gowrishankar M 			Creation of this test.
* *            <gowrishankar.m@in.ibm.com>
*
******************************************************************************/
#define _GNU_SOURCE 1
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "usctest.h"
#include "test.h"
#include <libclone.h>

char *TCID = "pidns12";
int TST_TOTAL = 1;
int errno;
int pipefd[2];

#define CHILD_PID       1
#define PARENT_PID      0

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 */
void cleanup()
{
	/* Clean the test testcase as LTP wants*/
	TEST_CLEANUP;

}

/*
 * child_signal_handler() - dummy function for sigaction()
 */
static void child_signal_handler(int sig, siginfo_t *si, void *unused)
{
	/* Recieved SIGUSR1. Check sender pid */
	if (si->si_pid == 0)
		tst_resm(TPASS, "cinit: signalling PID (from other namespace)"\
				" is 0 as expected");
	else
		tst_resm(TFAIL, "cinit: signalling PID (from other namespace)"\
				" is not 0, but %d.", si->si_pid);
}

/*
 * child_fn() - Inside container
 */
int child_fn(void *arg)
{
	struct sigaction sa;
	pid_t pid, ppid;

	/* Set process id and parent pid */
	pid = getpid();
	ppid = getppid();
	if (pid != CHILD_PID || ppid != PARENT_PID) {
		tst_resm(TBROK, "cinit: pidns is not created.");
		cleanup();
	}

	/* Close read end of pipe */
	close(pipefd[0]);

	/* Set signal handler for SIGUSR1 */
	sa.sa_flags = SA_SIGINFO;
	sigfillset(&sa.sa_mask);
	sa.sa_sigaction = child_signal_handler;
	if (sigaction(SIGUSR1, &sa, NULL) == -1) {
		tst_resm(TBROK, "cinit: sigaction() failed(%s).",\
				strerror(errno));
		cleanup();
	}

	/* Let parent to signal SIGUSR1 */
	if (write(pipefd[1], "c:go\0", 5) != 5) {
		tst_resm(TBROK, "cinit: pipe is broken to write");
		cleanup();
	}

	sleep(3);

	/* cleanup and exit */
	close(pipefd[1]);
	cleanup();

	/* Control won't reach below */
	exit(0);
}

/***********************************************************************
*   M A I N
***********************************************************************/

int main(int argc, char *argv[])
{
	int status;
	pid_t pid, cpid;
	char buf[5];

	pid = getpid();
	tst_resm(TINFO, "parent: PID is %d", pid);

	/* Create pipe for intercommunication */
	if (pipe(pipefd) == -1) {
		tst_resm(TBROK, "parent: pipe() failed. aborting!");
		cleanup();
	}

	cpid = ltp_clone_quick(CLONE_NEWPID|SIGCHLD, child_fn, NULL);
	if (cpid < 0) {
		tst_resm(TBROK, "parent: clone() failed(%s).",\
				strerror(errno));
		cleanup();
	}

	/* Close write end of pipe */
	close(pipefd[1]);

	/* Check if container is ready */
	read(pipefd[0], buf, 5);
	if (strcmp(buf, "c:go") != 0) {
		tst_resm(TBROK, "parent: container did not respond!");
		cleanup();
	}

	/* Send SIGUSR1 to container init */
	if (kill(cpid, SIGUSR1) == -1) {
		tst_resm(TBROK, "parent: kill() failed(%s).", strerror(errno));
		cleanup();
	}

	if (waitpid(cpid, &status, 0) < 0)
		tst_resm(TWARN, "parent: waitpid() failed(%s).",\
				strerror(errno));

	if (WIFSIGNALED(status) && WTERMSIG(status))
		tst_resm(TBROK, "child is terminated by signal(%s)",\
				strsignal(WTERMSIG(status)));

	/* Cleanup and exit */
	close(pipefd[0]);
	cleanup();

	/* Control won't reach below */
	exit(0);

}