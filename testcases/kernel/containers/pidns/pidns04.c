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
* FLAG DATE     	NAME           		        DESCRIPTION
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
#include <usctest.h>
#include <test.h>
#define CLEANUP cleanup
#include "libclone.h"

#define INIT_PID        1
#define CHILD_PID       1
#define PARENT_PID      0

char *TCID = "pid_namespace4";
int TST_TOTAL=1;
int     fd[2] ;

/*
 * child_fn1() - Inside container
*/
static int child_fn1(void *ttype)
{
	pid_t cpid, ppid;
	cpid = getpid();
	ppid = getppid();
	char mesg[] = "I was not killed !";
       	/* Child process closes up read side of pipe */
       	close(fd[0]);

	/* Comparing the values to make sure pidns is created correctly */
	if(( cpid == CHILD_PID) && ( ppid == PARENT_PID ) ) {
		tst_resm(TINFO, "PIDNS test is running inside container");
		kill(INIT_PID, SIGKILL);
		/* Verifying whether the container init is not killed, "
		 If so writing into the pipe created in the parent NS" */

        	/* Send "mesg" through the write side of pipe */
        	write(fd[1], mesg, (strlen(mesg)+1));
	}
	else {
		tst_resm(TFAIL, "got unexpected result of cpid=%d ppid=%d",
				cpid, ppid);
	}
	CLEANUP();
	close(fd[1]);
	tst_exit();
}

/***********************************************************************
*   M A I N
***********************************************************************/

int main(int argc, char *argv[])
{
	int ret, status, nbytes;
        char    readbuffer[80];

	pipe(fd);
	ret = do_clone_unshare_test(T_CLONE, CLONE_NEWPID, child_fn1, NULL);
	if ((wait(&status)) < 0) {
		tst_resm(TWARN, "wait() failed, skipping this test case");
		/* Cleanup & continue with next test case */
		CLEANUP();
	}
	if (ret == -1) {
		tst_resm(TFAIL, "clone() Failed, errno = %d :"
			" %s", ret, strerror(ret));
		/* Cleanup & continue with next test case */
		CLEANUP();
	}

	/* Parent process closes up write side of pipe */
	close(fd[1]);
	/* Read in a string from the pipe */
	nbytes = read(fd[0], readbuffer, sizeof(readbuffer));

	if (nbytes !=0 ) {
		tst_resm(TPASS, "Container init : %s", readbuffer);
	}
	else {
		tst_resm(TFAIL, "Container init is killed by SIGKILL !!!");
	}

	if (WTERMSIG(status)) {
		tst_resm(TFAIL, "Container init pid got killed by signal %d",
		WTERMSIG(status));
	}
        /* cleanup and exit */
	CLEANUP();
	close(fd[0]);

	tst_exit();

}	/* End main */

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 */
static void
cleanup()
{
	/* Clean the test testcase as LTP wants*/
	TEST_CLEANUP;
}
