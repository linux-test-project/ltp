/*
* Copyright (c) International Business Machines Corp., 2008
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
* * Assertion:
* * kill -USR1 container_init from outside a container
* * 	$ Where init has not defined a custom handler for USR1
* * 	$ Should kill the container
* * 	$ else the test fails.
* *
* * Description:
* *  This testcase creates container and waits till it is awakened by parent.
* *  The parent process sends the SIGUSR1 to container init.
* *  This would invoke default handler & terminate the container init.
* *  In parent process verify, if the SIGUSR1 signal is passed to c-init.
* *  If yes then Test is Passed else Test is Failed.
* *
* * History:
* *  DATE	  NAME				   DESCRIPTION
* *  14/11/08  Veerendra C  <vechandr@in.ibm.com> Verifying kill -USR1 in pidns
*
******************************************************************************/
#include "config.h"

#define _GNU_SOURCE 1
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <usctest.h>
#include <test.h>
#include <libclone.h>

#if defined(HAVE_SYS_CAPABILITY)
#include <sys/capability.h>

char *TCID = "pidns14";
int TST_TOTAL = 1;

int child_fn(void *);
void cleanup(void);

#define CHILD_PID	1
#define PARENT_PID	0

/*
 * child_fn() - Inside container
 */
int child_fn(void *ttype)
{
	pid_t pid, ppid;

	/* Set process id and parent pid */
	pid = getpid();
	ppid = getppid();

	if ((pid != CHILD_PID) || (ppid != PARENT_PID)) {
		tst_resm(TBROK, "pidns is not created.");
		cleanup();
	}
	pause();
	tst_resm(TFAIL, "Oops! Container init resumed after receiving SIGUSR1");
	return -1;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *			 completion or premature exit.
 */
void cleanup()
{
	/* Clean the test testcase as LTP wants*/
	TEST_CLEANUP;
	tst_exit();

}

/***********************************************************************
*   M A I N
***********************************************************************/

int main(int argc, char *argv[])
{
	int status;
	pid_t cpid;

	cpid = do_clone(CLONE_NEWPID | SIGCHLD, child_fn, NULL);

	if (cpid < 0) {
		tst_resm(TBROK, "clone() failed.");
		cleanup();
	}

	sleep(1);
	/* Passing the SIGUSR1 to the container init */
	if (kill(cpid, SIGUSR1) != 0) {
		tst_resm(TBROK, "kill(SIGUSR1) fails.");
		cleanup();
	}

	sleep(1);
	if (waitpid(cpid, &status, 0) < 0)
		tst_resm(TWARN, "waitpid() failed.");

	if ((WIFSIGNALED(status)) && (WTERMSIG(status) == SIGUSR1))
		tst_resm(TPASS, "Container init is killed as expected, "
				" when the SIGUSR1 is passed from parent\n");
	 else
		tst_resm(TFAIL, "After sending signal kill -USR1, "
				"returned unexpected error\n");

	return 0;
}	/* End main */

#else

char *TCID = "pidns14";
int TST_TOTAL = 0;              /* Total number of test cases. */

int
main()
{
    tst_resm(TBROK, "can't find header sys/capability.h");
    return 1;
}

#endif
