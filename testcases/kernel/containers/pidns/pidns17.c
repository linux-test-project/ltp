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
* File: pidns17.c
* *
* * Description:
* *  The pidns17.c testcase verifies inside the container, if kill(-1, SIGUSR1)
* *  terminates all children running inside.
* *
* * Test Assertion & Strategy:
* *  Create a PID namespace container.
* *  Spawn many children inside it.
* *  Invoke kill(-1, SIGUSR1) inside container and check if it terminates
* *  all children.
* *
* * Usage: <for command-line>
* *  pidns17
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
#include <libclone.h>

char *TCID = "pidns17";
int TST_TOTAL = 1;
int errno;

int child_fn(void *);
void cleanup(void);

#define CHILD_PID       1
#define PARENT_PID      0

/*
 * child_fn() - Inside container
 */
int child_fn(void *arg)
{
	pid_t pid, ppid;
	int i, children[10], status;

	/* Set process id and parent pid */
	pid = getpid();
	ppid = getppid();
	if (pid != CHILD_PID || ppid != PARENT_PID) {
		tst_resm(TBROK, "cinit: pidns is not created.");
		cleanup();
	}

	/* Spawn many children */
	for (i = 0; i < 10; i++)
		if ((children[i] = fork()) == 0)
			sleep(10);

	/* wait for last child to get scheduled */
	sleep(1);

	if (kill(-1, SIGUSR1) == -1) {
		tst_resm(TBROK, "cinit: kill(-1, SIGUSR1) failed");
		cleanup();
	}

	for (i = 0; i < 10; i++) {
		if (waitpid(children[i], &status, WNOHANG) == -1) {
			tst_resm(TBROK, "cinit: waitpid() failed(%s)",\
					strerror(errno));
			cleanup();
		}
		if (!(WIFSIGNALED(&status) && WTERMSIG(status) == SIGUSR1)) {
			tst_resm(TFAIL, "cinit: found a child alive still.");
			cleanup();
		}
	}
	tst_resm(TPASS, "cinit: all children are terminated.");

	/* cleanup and exit */
	cleanup();
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
		cleanup();
	}

	sleep(1);
	if (wait(&status) < 0)
		tst_resm(TWARN, "parent: waitpid() failed.");

	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		tst_resm(TWARN, "parent: container was terminated by %s",\
				strsignal(WTERMSIG(status)));

	/* cleanup and exit */
	cleanup();
	exit(0);
}	/* End main */

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
