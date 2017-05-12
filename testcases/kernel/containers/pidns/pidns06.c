/*
* Copyright (c) International Business Machines Corp., 2008
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
* This program is distributed in the hope that it will be useful
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
* the GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

*************************************************************************
* Description:
*  Testcase tries killing of the parent namespace pid by the container-init.
*  It also tries killing of non-existent PID, by the container-init.
*  Returns Success if Unable to kill, and proper error number is set.
*  else Returns Failure
*
* Steps:
* 1. Parent process clone a process with flag CLONE_NEWPID
* 2. The pid of the parent namespace is passed to the container.
* 3. Container receieves the PID and passes SIGKILL to this PID.
* 4. If kill() is unsuccessful and the errno is set to 'No Such process'
*	then sets PASS
*    else,
*	sets FAIL
* 5. It also verifies by passing SIGKILL to FAKE_PID
* 6. If kill() is unsuccessful and the errno is set to 'No Such process'
*	then sets PASS
*    else,
*	sets FAIL
*
*******************************************************************************/
#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include "pidns_helper.h"
#include "test.h"

#define CINIT_PID       1
#define PARENT_PID      0
#define FAKE_PID	-1

char *TCID = "pidns06";
int TST_TOTAL = 1;

/*
 * kill_pid_in_childfun()
 *      Cont-init tries to kill the parent-process using parent's global Pid.
 *	Also checks passing SIGKILL to non existent PID in the container.
 */
static int kill_pid_in_childfun(void *vtest)
{
	int cpid, ppid, *par_pid;
	int ret = 0;
	cpid = getpid();
	ppid = getppid();
	par_pid = (int *)vtest;

	/* Checking the values to make sure pidns is created correctly */
	if (cpid != CINIT_PID || ppid != PARENT_PID) {
		printf("Unexpected result for Container: init "
		       "pid=%d ppid=%d\n", cpid, ppid);
		exit(1);
	}

	/*
	 * While trying kill() of the pid of the parent namespace..
	 * Check to see if the errno was set to the expected, value of 3 : ESRCH
	 */
	ret = kill(*par_pid, SIGKILL);
	if (ret == -1 && errno == ESRCH) {
		printf("Container: killing parent pid=%d failed as expected "
		       "with ESRCH\n", *par_pid);
	} else {
		printf("Container: killing parent pid=%d, didn't fail as "
		       "expected with ESRCH (%d) and a return value of -1. Got "
		       "%d (\"%s\") and a return value of %d instead.\n",
		       *par_pid, ESRCH, errno, strerror(errno), ret);
		exit(1);
	}
	/*
	 * While killing non-existent pid in the container,
	 * Check to see if the errno was set to the expected, value of 3 : ESRCH
	 */
	ret = kill(FAKE_PID, SIGKILL);
	if (ret == -1 && errno == ESRCH) {
		printf("Container: killing non-existent pid failed as expected "
		       "with ESRCH\n");
	} else {
		printf("Container: killing non-existent pid, didn't fail as "
		       "expected with ESRCH (%d) and a return value of -1. Got "
		       "%d (\"%s\") and a return value of %d instead.\n",
		       ESRCH, errno, strerror(errno), ret);
		exit(1);
	}

	exit(0);
}

static void setup(void)
{
	tst_require_root();
	check_newpid();
}

int main(void)
{
	int status;

	setup();

	pid_t pid = getpid();

	tst_resm(TINFO, "Parent: Passing the pid of the process %d", pid);
	TEST(do_clone_unshare_test(T_CLONE, CLONE_NEWPID, kill_pid_in_childfun,
				   (void *)&pid));
	if (TEST_RETURN == -1) {
		tst_brkm(TFAIL | TERRNO, NULL, "clone failed");
	} else if (wait(&status) == -1) {
		tst_brkm(TFAIL | TERRNO, NULL, "wait failed");
	}

	tst_exit();
}
