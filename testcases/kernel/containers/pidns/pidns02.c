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

* File: pidns02.c
*
* Description:
*  The pidns02.c testcase builds into the ltp framework to verify
*  the basic functionality of PID Namespace.
*
* Verify that:
* 1. When parent clone a process with flag CLONE_NEWPID, the session ID of
* child should be always one.
*
* 2. When parent clone a process with flag CLONE_NEWPID, the parent process group ID
* should be always one.
*
* Total Tests
*
* Test Name: pidns02
*
* Test Assertion & Strategy:
*
* From main() clone a new child process with passing the clone_flag as CLONE_NEWPID,
* Call the setid() inside container.
* Inside the cloned pid check for the getsid(0) and getpgid(0)
* Verify with global macro defined value for parent pid & child pid.
*
* Usage: <for command-line>
* pidns02
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

char *TCID = "pid_namespace2";
int TST_TOTAL=1;

#define PGID  	1
#define SID	1

/*
 * child_fn1() - Inside container
 */
int child_fn1(void *vtest)
{
	pid_t pgid, sid;

	setsid();

	pgid = getpgid(0);
	sid  = getsid(0);

	tst_resm(TINFO, "Checking session id & group id inside container\n");
	if (pgid == PGID && sid == SID)
	{
                tst_resm(TPASS, "Success: Got Group ID = %d"
				" & Session ID = %d \n",pgid, sid);
	}
	else
		tst_resm(TFAIL, "Got unexpected result of"
			"Group ID = %d & Session ID = %d\n", pgid, sid);
	CLEANUP();
        return 0;
}

/***********************************************************************
*   M A I N
***********************************************************************/

int main(int argc, char *argv[])
{
	int ret, status;

	ret = do_clone_unshare_test(T_CLONE, CLONE_NEWPID, child_fn1, NULL);
	/* check return code */
	if (ret == -1) {
		tst_resm(TFAIL, "clone() Failed, errno = %d :"
			" %s", ret, strerror(ret));
		/* Cleanup & continue with next test case */
		CLEANUP();
	}

	/* Wait for child to finish */
	if ((wait(&status)) < 0) {
		tst_resm(TWARN | TERRNO, "wait() failed, skipping this "
					" test case");
		/* Cleanup & continue with next test case */
		CLEANUP();
	}

	if (WTERMSIG(status)) {
		tst_resm(TWARN, "child exited with signal %d",
			 WTERMSIG(status));
	}

        /* CLEANUP and exit */
	CLEANUP();
	tst_exit();

}	/* End main */

/*
 * cleanup() - performs all ONE TIME CLEANUP for this test at
 *             completion or premature exit.
 */
void
cleanup()
{
	/* Clean the test testcase as LTP wants*/
	TEST_CLEANUP;
}
