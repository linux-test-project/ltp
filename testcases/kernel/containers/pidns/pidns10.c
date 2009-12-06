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
* File: pidns10.c
* *
* * Description:
* *  The pidns10.c testcase verifies inside the container, if kill(-1, signal)
* *  fails with ESRCH when there are no processes in container.
* *
* * Test Assertion & Strategy:
* *  Create a PID namespace container.
* *  Invoke kill(-1, SIGUSR1) inside container and check return code and error.
* *  kill() should have failed;except swapper & init, no process is inside.
* *
* * Usage: <for command-line>
* *  pidns10
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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <usctest.h>
#include <test.h>
#define CLEANUP cleanup
#include "libclone.h"

char *TCID = "pidns10";
int TST_TOTAL = 1;
int errno;

int child_fn(void *);

#define CHILD_PID       1
#define PARENT_PID      0

/*
 * child_fn() - Inside container
 */
int child_fn(void *arg)
{
	pid_t pid, ppid;

	/* Set process id and parent pid */
	pid = getpid();
	ppid = getppid();
	if (pid != CHILD_PID || ppid != PARENT_PID) {
		tst_resm(TBROK, "cinit: pidns is not created.");
		CLEANUP();
	}

	if (kill(-1, SIGUSR1) != -1) {
		tst_resm(TFAIL, "cinit: kill(-1, sig) should have failed");
		CLEANUP();
	}

	if (errno == ESRCH)
		tst_resm(TPASS, "cinit: expected kill(-1, sig) failure.");
	else
		tst_resm(TFAIL, "cinit: kill(-1, sig) failure is not ESRCH, "
				"but %s", strerror(errno));

	/* CLEANUP and exit */
	CLEANUP();
	exit(0);
}

/***********************************************************************
*   M A I N
***********************************************************************/

int main(int argc, char *argv[])
{
	int status, ret;
	pid_t pid;

	pid = getpid();

	/* Container creation on PID namespace */
	ret = do_clone_unshare_test(T_CLONE,\
					CLONE_NEWPID, child_fn, NULL);
	if (ret != 0) {
		tst_resm(TBROK, "parent: clone() failed. rc=%d(%s)",\
				ret, strerror(errno));
		/* Cleanup & continue with next test case */
		CLEANUP();
	}

	sleep(1);
	if (wait(&status) < 0)
		tst_resm(TWARN, "parent: waitpid() failed.");

	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		tst_resm(TBROK, "parent: container was terminated by %s",\
				strsignal(WTERMSIG(status)));

	CLEANUP();
	exit(0);
}	/* End main */

/*
 * cleanup() - performs all ONE TIME CLEANUP for this test at
 *             completion or premature exit.
 */
static void cleanup()
{
	/* Clean the test testcase as LTP wants*/
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}
