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
* FLAG DATE		NAME			DESCRIPTION
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
#include "pidns_helper.h"
#include "test.h"

char *TCID = "pidns01";
int TST_TOTAL = 1;

#define CHILD_PID       1
#define PARENT_PID      0

/*
 * child_fn1() - Inside container
 */
int child_fn1(void *ttype LTP_ATTRIBUTE_UNUSED)
{
	int exit_val;
	pid_t cpid, ppid;
	cpid = getpid();
	ppid = getppid();

	tst_resm(TINFO, "PIDNS test is running inside container");
	if (cpid == CHILD_PID && ppid == PARENT_PID) {
		printf("Got expected cpid and ppid\n");
		exit_val = 0;
	} else {
		printf("Got unexpected result of cpid=%d ppid=%d\n",
		       cpid, ppid);
		exit_val = 1;
	}

	return exit_val;
}

static void setup(void)
{
	tst_require_root();
	check_newpid();
}

int main(int argc, char *argv[])
{
	int status;
	tst_parse_opts(argc, argv, NULL, NULL);
	setup();

	TEST(do_clone_unshare_test(T_CLONE, CLONE_NEWPID, child_fn1, NULL));

	if (TEST_RETURN == -1) {
		tst_brkm(TFAIL | TTERRNO, NULL, "clone failed");
	} else if ((wait(&status)) == -1) {
		tst_brkm(TWARN | TERRNO, NULL, "wait failed");
	}

	if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
		tst_resm(TFAIL, "child exited abnormally");
	else if (WIFSIGNALED(status)) {
		tst_resm(TFAIL, "child was killed with signal = %d",
			 WTERMSIG(status));
	}

	tst_exit();
}

