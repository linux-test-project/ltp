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

* File: pidns01.c
*
* Description:
*  The pidns01.c testcase builds into the ltp framework to verify
*  the basic functionality of PID Namespace.
*
* Verify that:
* 1. When parent clone a process with flag CLONE_NEWPID, the process ID of
* child should be always one.
*
* 2. When parent clone a process with flag CLONE_NEWPID, the parent process ID of
* should be always zero.
*
* Total Tests:
*
* Test Name: pidns01
*
* Test Assertion & Strategy:
*
* From main() clone a new child process with passing the clone_flag as CLONE_NEWPID,
* Inside the cloned pid check for the getpid() and getppid()
* Verify with global macro defined value for parent pid & child pid.
*
* Usage: <for command-line>
* pidns01
*
* History:
*
* FLAG DATE     	NAME           		DESCRIPTION
* 27/12/07  RISHIKESH K RAJAK <risrajak@in.ibm.com> Created this test
*
*******************************************************************************************/
#define _GNU_SOURCE
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

char *TCID = "pid_namespace1";
int TST_TOTAL=1;

#define CHILD_PID       1
#define PARENT_PID      0

/*
 * child_fn1() - Inside container
 */
int child_fn1(void *ttype)
{
	pid_t cpid, ppid;
	cpid = getpid();
	ppid = getppid();

	tst_resm(TINFO, "PIDNS test is running inside container\n");
	if(( cpid == CHILD_PID) &&
		( ppid == PARENT_PID ) )
	{
                tst_resm(TPASS, "Success:" );
	}
	else
	{
		tst_resm(TFAIL, "FAIL: Got unexpected result of"
			" cpid=%d ppid=%d\n", cpid, ppid);
	}
	cleanup();

	/* NOT REACHED */
	return 0;
}

/***********************************************************************
*   M A I N
***********************************************************************/

int main(int argc, char *argv[])
{
	int ret, status;

	ret = do_clone_unshare_test(T_CLONE,
				CLONE_NEWPID, child_fn1, NULL);

	/* check return code */
	if (ret == -1) {
		tst_resm(TFAIL, "clone() Failed, errno = %d :"
			" %s", ret, strerror(ret));
		/* Cleanup & continue with next test case */
		cleanup();
	}

	/* Wait for child to finish */
	if ((wait(&status)) < 0) {
		tst_resm(TWARN, "wait() failed, skipping this"
			" test case");
		/* Cleanup & continue with next test case */
		cleanup();
	}

	if (WTERMSIG(status)) {
		tst_resm(TWARN, "child exited with signal %d",
			 WTERMSIG(status));
	}

        /* cleanup and exit */
	cleanup();

	/*NOTREACHED*/
	return 0;

}	/* End main */

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 */
static void cleanup()
{
	/* Clean the test testcase as LTP wants */
	TEST_CLEANUP;
	/* exit with return code appropriate for results */
	tst_exit();
}
