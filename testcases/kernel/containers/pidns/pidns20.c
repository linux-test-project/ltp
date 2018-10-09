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
#include "pidns_helper.h"
#include "test.h"
#include "safe_macros.h"

char *TCID = "pidns20";
int TST_TOTAL = 1;

int parent_cinit[2];
int cinit_parent[2];
int broken = 1;			/* broken should be 0 when test completes properly */

#define CHILD_PID       1
#define PARENT_PID      0

/*
 * child_signal_handler() - to handle SIGUSR1
 */
static void child_signal_handler(int sig, siginfo_t * si, void *unused)
{
	if (si->si_signo != SIGUSR1)
		tst_resm(TBROK, "cinit: received %s unexpectedly!",
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
		printf("cinit: pidns was not created properly\n");
		exit(1);
	}

	/* Setup pipes to communicate with parent */
	close(cinit_parent[0]);
	close(parent_cinit[1]);

	/* Block SIGUSR1 signal */
	sigemptyset(&newset);
	sigaddset(&newset, SIGUSR1);
	if (sigprocmask(SIG_BLOCK, &newset, 0) == -1) {
		perror("cinit: sigprocmask() failed");
		exit(1);
	}
	tst_resm(TINFO, "cinit: blocked SIGUSR1");

	/* Let parent to queue SIGUSR1 in pending */
	if (write(cinit_parent[1], "c:go", 5) != 5) {
		perror("cinit: pipe is broken to write");
		exit(1);
	}

	/* Check if parent has queued up SIGUSR1 */
	read(parent_cinit[0], buf, 5);
	if (strcmp(buf, "p:go") != 0) {
		printf("cinit: parent did not respond!\n");
		exit(1);
	}

	/* Now redefine handler for SIGUSR1 */
	sa.sa_flags = SA_SIGINFO;
	sigfillset(&sa.sa_mask);
	sa.sa_sigaction = child_signal_handler;
	if (sigaction(SIGUSR1, &sa, NULL) == -1) {
		perror("cinit: sigaction failed");
		exit(1);
	}

	/* Unblock traffic on SIGUSR1 queue */
	tst_resm(TINFO, "cinit: unblocking SIGUSR1");
	sigprocmask(SIG_UNBLOCK, &newset, 0);

	/* Check if new handler is called */
	if (broken == 1) {
		printf("cinit: broken flag didn't change\n");
		exit(1);
	}

	/* Cleanup and exit */
	close(cinit_parent[1]);
	close(parent_cinit[0]);
	exit(0);
}

static void setup(void)
{
	tst_require_root();
	check_newpid();
}

int main(int argc, char *argv[])
{
	int status;
	char buf[5];
	pid_t cpid;

	setup();

	/* Create pipes for intercommunication */
	if (pipe(parent_cinit) == -1 || pipe(cinit_parent) == -1) {
		tst_brkm(TBROK | TERRNO, NULL, "pipe failed");
	}

	cpid = ltp_clone_quick(CLONE_NEWPID | SIGCHLD, child_fn, NULL);
	if (cpid == -1) {
		tst_brkm(TBROK | TERRNO, NULL, "clone failed");
	}

	/* Setup pipe read and write ends */
	close(cinit_parent[1]);
	close(parent_cinit[0]);

	/* Is container ready */
	read(cinit_parent[0], buf, 5);
	if (strcmp(buf, "c:go") != 0) {
		tst_brkm(TBROK, NULL, "parent: container did not respond!");
	}

	/* Enqueue SIGUSR1 in pending signal queue of container */
	SAFE_KILL(NULL, cpid, SIGUSR1);

	tst_resm(TINFO, "parent: signalled SIGUSR1 to container");
	if (write(parent_cinit[1], "p:go", 5) != 5) {
		tst_brkm(TBROK | TERRNO, NULL, "write failed");
	}

	/* collect exit status of child */
	SAFE_WAIT(NULL, &status);

	if (WIFSIGNALED(status)) {
		if (WTERMSIG(status) == SIGUSR1)
			tst_resm(TFAIL,
				 "user function was not called inside cinit");
		else
			tst_resm(TBROK,
				 "cinit was terminated by %d",
				 WTERMSIG(status));
	}

	/* Cleanup and exit */
	close(parent_cinit[1]);
	close(cinit_parent[0]);
	tst_exit();
}
