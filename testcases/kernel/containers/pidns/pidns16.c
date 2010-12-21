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

* * Test Assertion.
* *----------------
* * kill -USR1 container_init
* *	- from the parent process and also inside a container
* *	- Where init has defined a custom handler for USR1
* *	- Should call the handler and
* * 	- Verify whether the signal handler is called from the proper process.
* *
* * Description:
* *  Create PID namespace container.
* *  Container init defines the handler for SIGUSR1 and waits indefinetly.
* *  Parent sends SIGUSR1 to container init.
* *  The signal handler is handled and the cont-init resumes normally.
* *  From the container, again the signal SIGUSR1 is sent.
* *  In the sig-handler check if it's invoked from correct pid(parent/container)
* *  If cont-init wakes up properly -
* *  it will return expected value at exit which is verified at the end.
* *
* * History:
* *  DATE	  NAME				   DESCRIPTION
* *  04/11/08  Veerendra C  <vechandr@in.ibm.com> Verifying cont init kill -USR1
*
*******************************************************************************/
#include "config.h"

#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include "usctest.h"
#include "test.h"
#include <libclone.h>
#define CHILD_PID	1
#define PARENT_PID	0

char *TCID = "pidns16";
int TST_TOTAL = 1;
pid_t globalpid;

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *			 completion or premature exit.
 */
void cleanup()
{
	/* Clean the test testcase as LTP wants*/
	TEST_CLEANUP;

}

void child_signal_handler(int sig, siginfo_t *si, void *unused)
{
	static int c = 1;
	/* Verifying from which process the signal handler is signalled */

	if ((c == 1) && (si->si_pid == globalpid))
		tst_resm(TINFO, "sig_handler is signalled from pid  %d" ,
				globalpid);
	else if ((c == 2) && (si->si_pid == CHILD_PID))
		tst_resm(TINFO, "sig_handler is signalled from pid  %d" ,
				CHILD_PID);
	else
		tst_resm(TBROK, "Unexpected value for Sending-ProcessID"
				" when signal handler called %d\n", si->si_pid);
	c++;
}

/*
 * child_fn() - Inside container
 */
int child_fn(void *ttype)
{
	struct sigaction sa;
	pid_t pid, ppid;

	/* Set process id and parent pid */
	pid = getpid();
	ppid = getppid();

	if ((pid != CHILD_PID) || (ppid != PARENT_PID))
		tst_resm(TBROK, "pidns is not created.");

	/* Set signal handler for SIGUSR1, also mask other signals */
	sa.sa_flags = SA_SIGINFO;
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = child_signal_handler;
	if (sigaction(SIGUSR1, &sa, NULL) == -1)
		tst_resm(TBROK, "%d: sigaction() failed", pid);

	pause();
	tst_resm(TINFO, "Container: Resumed after receiving SIGUSR1 "
			"from parentNS ");
	if (kill(pid, SIGUSR1) != 0) {
		tst_resm(TFAIL, "kill(SIGUSR1) fails.");
		cleanup();
	}
	tst_resm(TINFO, "Container: Resumed after sending SIGUSR1 "
			"from container itself");
	_exit(10);
}

/***********************************************************************
*   M A I N
***********************************************************************/
int main(int argc, char *argv[])
{
	int status;
	pid_t cpid;

	globalpid = getpid();

	cpid = ltp_clone_quick(CLONE_NEWPID | SIGCHLD, child_fn, NULL);

	if (cpid < 0) {
		tst_resm(TBROK, "clone() failed.");
		cleanup();
	}

	sleep(1);
	if (kill(cpid, SIGUSR1) != 0) {
		tst_resm(TFAIL, "kill(SIGUSR1) fails.");
		cleanup();
	}
	sleep(1);
	if (waitpid(cpid, &status, 0) < 0)
		tst_resm(TWARN, "waitpid() failed.");

	if ((WIFEXITED(status)) && (WEXITSTATUS(status) == 10))
		tst_resm(TPASS, "container init continued successfuly, "
			"after handling signal -USR1\n");
	 else
		tst_resm(TFAIL, "c-init failed to continue after "
				"passing kill -USR1");
	cleanup();
	tst_exit();
}