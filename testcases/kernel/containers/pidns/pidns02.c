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

* File: pidns02.c
*
* Description:
*	The pidns02.c testcase builds into the ltp framework to verify
*	the basic functionality of PID Namespace.
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
*/

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

char *TCID = "pidns02";
int TST_TOTAL = 1;

#define PGID	1
#define SID	1

/*
 * child_fn1() - Inside container
 */
int child_fn1(void *vtest)
{
	pid_t pgid, sid;

	setsid();

	pgid = getpgid(0);
	sid = getsid(0);

	printf("Checking session id & group id inside container\n");
	if (pgid == PGID && sid == SID) {
		printf("Success: Got Group ID = %d & Session ID = %d\n",
		       pgid, sid);
		exit(0);
	} else {
		printf("Got unexpected result of Group ID = %d & Session ID = "
		       "%d\n", pgid, sid);
		exit(1);
	}
}

static void setup(void)
{
	tst_require_root();
	check_newpid();
}

int main(int argc, char *argv[])
{
	int status;

	setup();

	TEST(do_clone_unshare_test(T_CLONE, CLONE_NEWPID, child_fn1, NULL));
	if (TEST_RETURN == -1) {
		tst_brkm(TFAIL | TTERRNO, NULL, "clone failed");
	} else if ((wait(&status)) == -1) {
		tst_brkm(TFAIL | TERRNO, NULL, "wait failed");
	}

	if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
		tst_resm(TFAIL | TERRNO, "child exited abnormally");
	} else if (WIFSIGNALED(status)) {
		tst_resm(TFAIL | TERRNO, "child exited with signal %d",
			 WTERMSIG(status));
	}

	tst_exit();

}
