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
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

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
* History:
*
* FLAG DATE     	NAME				Description.
* 21/10/08  	Veerendra C <vechandr@in.ibm.com> Verifies killing of processes
*							in container.
*******************************************************************************/
#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <usctest.h>
#include <test.h>
#include <libclone.h>
#include <signal.h>

#define CINIT_PID       1
#define PARENT_PID      0
#define FAKE_PID	1230

char *TCID = "pidns06";
int TST_TOTAL = 1;

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	     completion or premature exit.
 */
void cleanup()
{
	/* Clean the test testcase as LTP wants*/
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}

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
	if ((cpid != CINIT_PID) || (ppid != PARENT_PID)) {
		tst_resm(TFAIL, "Unexpected result for Container: "
				" init pid=%d parent pid=%d\n", cpid, ppid);
		cleanup();
	}

	/*
	* While trying kill() of the pid of the parent namespace..
	* Check to see if the errno was set to the expected, value of 3 : ESRCH
	*/
	ret = kill(*par_pid, SIGKILL);
	if (ret == -1 && errno == ESRCH) {
		tst_resm(TPASS, "Container: tried kill() on the parent "
			 "pid %d: errno set to %d (%s), as expected\n",
			 *par_pid, errno , strerror(errno));
	} else {
		tst_resm(TFAIL, "Container: tried kill() on the parent "
			"pid %d, errno set to %d, (%s), expected %d, (%s). \n"
			"\t\t\t\tReturn value is %d, expected -1.\n" ,
			*par_pid, errno , strerror(errno), 3, strerror(3), ret);
	}
	/*
	* While killing non-existent pid in the container,
	* Check to see if the errno was set to the expected, value of 3 : ESRCH
	*/
	ret = kill(FAKE_PID, SIGKILL);
	if (ret == -1 && errno == ESRCH) {
		tst_resm(TPASS, "Container: While killing non existent pid"
				" errno set to %d : %s, as expected\n" ,
				errno , strerror(errno));
	} else {
		tst_resm(TFAIL, "Container: While killing non-existent pid"
				" errno set to %d : %s expected %d : %s. \n"
				"\t\t\t\tReturn value is %d, expected -1.\n" ,
				errno, strerror(errno), 3, strerror(3), ret);
	}

	cleanup();

	/* NOT REACHED */
	return 0;
}

/*********************************************************************
*   M A I N
***********************************************************************/

int main()
{
	int ret, status;
	pid_t pid = getpid();

	tst_resm(TINFO, "Parent: Passing the pid of the process %d", pid);
	ret = do_clone_unshare_test(T_CLONE, CLONE_NEWPID,
					kill_pid_in_childfun, (void *) &pid);

	if (ret == -1) {
		tst_resm(TFAIL, "clone() Failed, errno = %d : %s\n" ,
			 ret, strerror(ret));
		cleanup();
	}

	/* Wait for child to finish */
	if ((wait(&status)) < 0) {
		tst_resm(TWARN, "wait() failed, skipping this test case");
		cleanup();
	}

	/* cleanup and exit */
	cleanup();

	/*NOTREACHED*/
	return 0;
}

