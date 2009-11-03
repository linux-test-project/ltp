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
* File: pidns20.c
* *
* * Description:
* *  The pidns20.c testcase verifies that signal handler of SIGUSR1 is called
* *  (and cinit is NOT terminated) when:
* *    - container-init blocks SIGUSR1,
* *    - parent queues SIGUSR1 and
* *    - a handler is specified for SIGUSR1 before it is unblocked.
* *
* * Test Assertion & Strategy:
* *  Create a PID namespace container.
* *  Block SIGUSR1 signal inside it.
* *  Let parent to deliver SIGUSR1 signal to container.
* *  Redefine SIGUSR1 handler of cinit to user function.
* *  Unblock SIGUSR1 from blocked queue.
* *  Check if user function is called.
* *
* * Usage: <for command-line>
* *  pidns20
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
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <usctest.h>
#include <test.h>
#include <libclone.h>

char *TCID = "pidns20";
int TST_TOTAL = 1;

int errno;
int parent_cinit[2];
int cinit_parent[2];
int broken = 1; /* broken should be 0 when test completes properly */

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

	/* exit with return code appropriate for results */
	tst_exit();
}

/*
 * child_signal_handler() - to handle SIGUSR1
 */
static void child_signal_handler(int sig, siginfo_t *si, void *unused)
{
	if (si->si_signo != SIGUSR1)
		tst_resm(TBROK, "cinit: recieved %s unexpectedly!",\
				strsignal(si->si_signo));
	else
		tst_resm(TPASS, "cinit: user function is called as expected");

	/* Disable broken flag */
	broken = 0;
}

/*
 * child_fn() - Inside container
 */
int child_fn(void *arg)
{
	pid_t pid, ppid;
	sigset_t newset;
	struct sigaction sa;
	char buf[5];

	/* Setup pipe read and write ends */
	pid = getpid();
	ppid = getppid();

	if (pid != CHILD_PID || ppid != PARENT_PID) {
		tst_resm(TBROK, "cinit: pidns is not created");
		cleanup();
	}

	/* Setup pipes to communicate with parent */
	close(cinit_parent[0]);
	close(parent_cinit[1]);

	/* Block SIGUSR1 signal */
	sigemptyset(&newset);
	sigaddset(&newset, SIGUSR1);
	if (sigprocmask(SIG_BLOCK, &newset, 0) == -1) {
		tst_resm(TBROK, "cinit: sigprocmask() failed(%s)",\
				strerror(errno));
		cleanup();
	}
	tst_resm(TINFO, "cinit: blocked SIGUSR1");

	/* Let parent to queue SIGUSR1 in pending */
	if (write(cinit_parent[1], "c:go", 5) != 5) {
		tst_resm(TBROK, "cinit: pipe is broken to write(%s)",\
				strerror(errno));
		cleanup();
	}

	/* Check if parent has queued up SIGUSR1 */
	read(parent_cinit[0], buf, 5);
	if (strcmp(buf, "p:go") != 0) {
		tst_resm(TBROK, "cinit: parent did not respond!");
		cleanup();
	}

	/* Now redefine handler for SIGUSR1 */
	sa.sa_flags = SA_SIGINFO;
	sigfillset(&sa.sa_mask);
	sa.sa_sigaction = child_signal_handler;
	if (sigaction(SIGUSR1, &sa, NULL) == -1) {
		tst_resm(TBROK, "cinit: sigaction() failed(%s)",\
				strerror(errno));
		cleanup();
	}

	/* Unblock traffic on SIGUSR1 queue */
	tst_resm(TINFO, "cinit: unblocking SIGUSR1");
	sigprocmask(SIG_UNBLOCK, &newset, 0);

	/* Check if new handler is called */
	if (broken == 1) {
		tst_resm(TBROK, "cinit: test is broken for unknown reason");
		cleanup();
	}

	/* Cleanup and exit */
	close(cinit_parent[1]);
	close(parent_cinit[0]);
	cleanup();
	exit(0);
}


/***********************************************************************
*   M A I N
***********************************************************************/

int main(int argc, char *argv[])
{
	int status;
	char buf[5];
	pid_t cpid;

	/* Create pipes for intercommunication */
	if (pipe(parent_cinit) == -1 || pipe(cinit_parent) == -1) {
		tst_resm(TBROK, "parent: pipe() failed. aborting!");
		cleanup();
	}

	cpid = ltp_clone_quick(CLONE_NEWPID|SIGCHLD, child_fn, NULL);
	if (cpid < 0) {
		tst_resm(TBROK, "parent: clone() failed(%s)",\
				strerror(errno));
		cleanup();
	}

	/* Setup pipe read and write ends */
	close(cinit_parent[1]);
	close(parent_cinit[0]);

	/* Is container ready */
	read(cinit_parent[0], buf, 5);
	if (strcmp(buf, "c:go") != 0) {
		tst_resm(TBROK, "parent: container did not respond!");
		cleanup();
	}

	/* Enqueue SIGUSR1 in pending signal queue of container */
	if (kill(cpid, SIGUSR1) == -1) {
		tst_resm(TBROK, "parent: kill() failed(%s)",\
				strerror(errno));
		cleanup();
	}

	tst_resm(TINFO, "parent: signalled SIGUSR1 to container");
	if (write(parent_cinit[1], "p:go", 5) != 5) {
		tst_resm(TBROK, "parent: pipe is broken to write(%s)",\
				strerror(errno));
		cleanup();
	}

	/* collect exit status of child */
	if (wait(&status) == -1) {
		tst_resm(TBROK, "parent: wait() failed(%s)",\
				strerror(errno));
		cleanup();
	}

	if (WIFSIGNALED(status)) {
		if (WTERMSIG(status) == SIGUSR1)
			tst_resm(TFAIL, "parent: user function is not called "\
					"inside cinit");
		else
			tst_resm(TBROK, "parent: cinit is terminated by %i",\
					WTERMSIG(status));
	}

	/* Cleanup and exit */
	close(parent_cinit[1]);
	close(cinit_parent[0]);
	cleanup();
	exit(0);
}	/* End main */

