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
#include "pidns_helper.h"
#include "test.h"

char *TCID = "pidns17";
int TST_TOTAL = 1;

int child_fn(void *);

#define CHILD_PID       1
#define PARENT_PID      0

/*
 * child_fn() - Inside container
 */
int child_fn(void *arg)
{
	int children[10], exit_val, i, status;
	pid_t pid, ppid;

	/* Set process id and parent pid */
	pid = getpid();
	ppid = getppid();
	if (pid != CHILD_PID || ppid != PARENT_PID) {
		printf("cinit: pidns was not created\n");
		exit(1);
	}

	exit_val = 0;

	/* Spawn many children */
	for (i = 0; i < ARRAY_SIZE(children); i++) {
		switch ((children[i] = fork())) {
		case -1:
			perror("fork failed");
			exit_val = 1;
			break;
		case 0:
			pause();
			/* XXX (garrcoop): why exit with an exit code of 2? */
			exit(2);
			break;
		default:
			/* fork succeeded. */
			break;
		}
	}
	/* wait for last child to get scheduled */
	sleep(1);

	if (kill(-1, SIGUSR1) == -1) {
		perror("cinit: kill(-1, SIGUSR1) failed");
		exit_val = 1;
	}

	for (i = 0; i < ARRAY_SIZE(children); i++) {
		if (waitpid(children[i], &status, 0) == -1) {
			perror("cinit: waitpid failed");
			kill(children[i], SIGTERM);
			waitpid(children[i], &status, 0);
			exit_val = 1;
		}
		if (!(WIFSIGNALED(status) || WTERMSIG(status) == SIGUSR1)) {
			/*
			 * XXX (garrcoop): this status reporting is overly
			 * noisy. Someone obviously needs to read the
			 * constraints documented in wait(2) a bit more
			 * closely -- in particular the relationship between
			 * WIFEXITED and WEXITSTATUS, and WIFSIGNALED and
			 * WTERMSIG.
			 */
			printf("cinit: found a child alive still "
			       "%d exit: %d, %d, signal %d, %d", i,
			       WIFEXITED(status), WEXITSTATUS(status),
			       WIFSIGNALED(status), WTERMSIG(status));
			exit_val = 1;
		}
	}
	if (exit_val == 0)
		printf("cinit: all children have terminated.\n");

	exit(exit_val);
}

static void setup(void)
{
	tst_require_root();
	check_newpid();
}

int main(int argc, char *argv[])
{
	int status;
	pid_t pid;

	setup();

	pid = getpid();

	/* Container creation on PID namespace */
	TEST(do_clone_unshare_test(T_CLONE, CLONE_NEWPID, child_fn, NULL));
	if (TEST_RETURN == -1) {
		tst_brkm(TBROK | TERRNO, NULL, "clone failed");
	}

	sleep(1);
	if (wait(&status) == -1)
		tst_resm(TFAIL, "waitpid failed");

	if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
		tst_resm(TFAIL, "container exited abnormally");
	else if (WIFSIGNALED(status))
		tst_resm(TFAIL,
			 "container was signaled with signal = %d",
			 WTERMSIG(status));

	tst_exit();

}
