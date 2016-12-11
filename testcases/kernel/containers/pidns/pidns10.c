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
#include "pidns_helper.h"
#include "test.h"

char *TCID = "pidns10";
int TST_TOTAL = 1;

int child_fn(void *);

#define CHILD_PID       1
#define PARENT_PID      0

/*
 * child_fn() - Inside container
 */
int child_fn(void *arg)
{
	int exit_val, ret;
	pid_t pid, ppid;

	/* Set process id and parent pid */
	pid = getpid();
	ppid = getppid();
	if (pid != CHILD_PID || ppid != PARENT_PID) {
		printf("cinit: pidns was not created.\n");
		return 1;
	}

	if ((ret = kill(-1, SIGUSR1)) == -1 && errno == ESRCH) {
		printf("cinit: kill(-1, sig) failed with -1 / ESRCH as "
		       "expected\n");
		exit_val = 0;
	} else {
		printf("cinit: kill(-1, sig) didn't fail with -1 / ESRCH "
		       "(%d); failed with %d / %d instead", ESRCH, ret, errno);
		exit_val = 1;
	}
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
		tst_brkm(TBROK | TTERRNO, NULL, "clone failed");
	}

	sleep(1);
	if (wait(&status) < 0)
		tst_resm(TWARN, "parent: waitpid() failed.");

	if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
		tst_resm(TBROK, "container was terminated abnormally");

	tst_exit();
}
