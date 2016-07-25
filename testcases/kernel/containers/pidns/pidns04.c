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

* File: pidns04.c
*
* Description:
*  The pidns04.c testcase builds into the ltp framework to verify
*  the basic functionality of PID Namespace.
*
* Verify that:
* 1. When parent clone a process with flag CLONE_NEWPID, the process ID of
* child should be one.
*
* 2. When parent clone a process with flag CLONE_NEWPID, the parent process ID
* of should be zero.
*
* 3. The container init process (one), should not get killed by the SIGKILL in
* the childNS
*
* Total Tests:
*
* Test Name: pidns04
*
* Test Assertion & Strategy:
*
* From main() clone a new child process with passing the clone_flag as
* CLONE_NEWPID.
* The container init, should not get killed by the SIGKILL inside the child NS.
* Usage: <for command-line>
* pidns04
*
* History:
*
* FLAG DATE     	NAME	   			DESCRIPTION
* 08/10/08      Veerendra C <vechandr@in.ibm.com> Verifies killing of cont init.
*
*******************************************************************************/
#define _GNU_SOURCE 1
#include <sys/wait.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#define CLEANUP cleanup
#include "pidns_helper.h"
#include "test.h"

#define INIT_PID	1
#define CHILD_PID       1
#define PARENT_PID      0

char *TCID = "pidns04";
int TST_TOTAL = 1;
int fd[2];

/*
 * child_fn1() - Inside container
*/
static int child_fn1(void *ttype)
{
	int exit_val;
	pid_t cpid, ppid;
	cpid = getpid();
	ppid = getppid();
	char mesg[] = "I was not killed !";
	/* Child process closes up read side of pipe */
	close(fd[0]);

	/* Comparing the values to make sure pidns is created correctly */
	if ((cpid == CHILD_PID) && (ppid == PARENT_PID)) {
		printf("PIDNS test is running inside container\n");
		kill(INIT_PID, SIGKILL);
		/* Verifying whether the container init is not killed, "
		   If so writing into the pipe created in the parent NS" */

		/* Send "mesg" through the write side of pipe */
		write(fd[1], mesg, (strlen(mesg) + 1));
		exit_val = 0;
	} else {
		printf("got unexpected result of cpid=%d ppid=%d\n",
		       cpid, ppid);
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
	int nbytes, status;
	char readbuffer[80];

	setup();

	pipe(fd);
	TEST(do_clone_unshare_test(T_CLONE, CLONE_NEWPID, child_fn1, NULL));
	if (TEST_RETURN == -1) {
		tst_brkm(TFAIL | TTERRNO, CLEANUP, "clone failed");
	} else if (wait(&status) == -1) {
		tst_brkm(TFAIL | TERRNO, CLEANUP, "wait failed");
	}

	/* Parent process closes up write side of pipe */
	close(fd[1]);
	/* Read in a string from the pipe */
	nbytes = read(fd[0], readbuffer, sizeof(readbuffer));

	if (0 <= nbytes) {
		tst_resm(TPASS, "Container init : %s", readbuffer);
	} else {
		tst_brkm(TFAIL, CLEANUP,
			 "Container init is killed by SIGKILL !!!");
	}

	if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
		tst_resm(TFAIL, "Container init pid exited abnormally");
	} else if (WIFSIGNALED(status)) {
		tst_resm(TFAIL, "Container init pid got killed by signal %d",
			 WTERMSIG(status));
	}
	CLEANUP();

	tst_exit();

}

static void cleanup(void)
{
	close(fd[0]);
}
