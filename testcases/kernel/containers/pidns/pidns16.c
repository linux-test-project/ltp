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

* * Test Assertion.
* *----------------
* * kill -USR1 container_init
* *	- from the parent process and also inside a container
* *	- Where init has defined a custom handler for USR1
* *	- Should call the handler and
* *	- Verify whether the signal handler is called from the proper process.
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
#include "pidns_helper.h"
#include "test.h"

#define CHILD_PID	1
#define PARENT_PID	0

char *TCID = "pidns16";
int TST_TOTAL = 3;

void child_signal_handler(int sig, siginfo_t * si, void *unused)
{
	static int c = 1;
	pid_t expected_pid;

	/* Verifying from which process the signal handler is signalled */

	switch (c) {
	case 1:
		expected_pid = PARENT_PID;
		break;
	case 2:
		expected_pid = CHILD_PID;
		break;
	default:
		tst_resm(TBROK, "child should NOT be signalled 3+ times");
		return;
	}

	if (si->si_pid == expected_pid)
		tst_resm(TPASS, "child is signalled from expected pid %d",
			 expected_pid);
	else
		tst_resm(TFAIL, "child is signalled from unexpected pid %d,"
			 " expecting pid %d", si->si_pid, expected_pid);

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
	}
	tst_resm(TINFO, "Container: Resumed after sending SIGUSR1 "
		 "from container itself");
	_exit(10);
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
	pid_t cpid;

	setup();

	cpid = ltp_clone_quick(CLONE_NEWPID | SIGCHLD, child_fn, NULL);

	if (cpid < 0) {
		tst_resm(TBROK, "clone() failed.");
	}

	sleep(1);
	if (kill(cpid, SIGUSR1) != 0) {
		tst_resm(TFAIL, "kill(SIGUSR1) fails.");
	}
	sleep(1);
	if (waitpid(cpid, &status, 0) < 0)
		tst_resm(TWARN, "waitpid() failed.");

	if ((WIFEXITED(status)) && (WEXITSTATUS(status) == 10))
		tst_resm(TPASS, "container init continued successfuly, "
			 "after handling signal -USR1");
	else
		tst_resm(TFAIL, "c-init failed to continue after "
			 "passing kill -USR1");
	tst_exit();
}
