/*
* Copyright (c) International Business Machines Corp., 2007
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
* the GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
***************************************************************************

* File: pidns03.c
*
* Description:
*  The pidns03.c testcase builds into the ltp framework to verify
*  the basic functionality of PID Namespace.
*
* Verify that:
* 1. When parent, clone a process with flag CLONE_NEWPID, see the process id
* of the parent is existing after mounting /proc
*
* Total Tests:
*
* Test Name: pidns03
*
* Test Assertion & Strategy:
*
* From main() clone a new child process with passing the clone_flag as CLONE_NEWPID,
* Pass the main() process id as an argument of cloned function.
* mount the /proc directory inside container
* Verify with checking /proc/arg1 directory inside container, it should see after mounting
* /proc but it the actual value of parent process id is zero.
*
* Usage: <for command-line>
* pidns03
*
* History:
*
* FLAG DATE     	NAME           		DESCRIPTION
* 27/12/07  RISHIKESH K RAJAK <risrajak@in.ibm.com> Created this test
*
*******************************************************************************************/
#include <sys/wait.h>
#include <sys/mount.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <usctest.h>
#include <test.h>
#include <libclone.h>

char *TCID = "pid_namespace3";
int TST_TOTAL;

static void cleanup();
static int child_fn();

/***********************************************************************
*   M A I N
***********************************************************************/
int
main(argc, argv)
int argc;
char **argv;
{
	int ret,status;
	pid_t ppid;

	/* Store the value of parent process ID  and pass them as argument */
	ppid = getpid();

	/* Create a Container and execute to test the functionality */
	ret = do_clone_unshare_test(T_CLONE, CLONE_NEWPID|CLONE_NEWNS, child_fn, &ppid);

	/* check return code */
	if (ret == -1) {
		tst_resm(TFAIL, "clone() Failed, errno = %d :"
			" %s", ret,
		strerror(ret));
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
 * child_fn() - child function
 */

int
child_fn(pid_t *Ppid)
{
	char dirnam[50];
	DIR *d;
	pid_t parent_pid, cloned_pid;

	parent_pid = getppid();
	cloned_pid = getpid();

	tst_resm(TINFO, " Checking pid for parent ns and container-init\n"
			"\t\t\t\tParent namespace pid = %d,"
			"container parent pid = %d,"
			"and container pid = %d\n",
			*Ppid, parent_pid, cloned_pid);

	/* do any /proc setup which winds up being necessary. */
	if (mount("proc", "/proc", "proc", 0, NULL) < 0)
		tst_resm(TFAIL, "mount failed : \n");

	/* Check for the parent pid is existing still? */
	sprintf(dirnam, "/proc/%d", *Ppid);

	d = opendir(dirnam);
	if (!d) {
		tst_resm(TPASS, \
		"Got the proc file directory created by parent ns %d\n", *Ppid);
		umount("/proc");
	} else {
		tst_resm(TFAIL, "Failed to open /proc directory \n");
		closedir(d);
	}

	cleanup();

	/* NOT REACHED */
	return 0;
}

/*
 *cleanup() -  performs all ONE TIME cleanup for this test at
 *              completion or premature exit.
 */
void
cleanup()
{

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();

}       /* End cleanup() */
