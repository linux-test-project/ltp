/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */
/* 10/30/2002	Port to LTP	dbarrera@us.ibm.com */

/*
 * NAME
 *	semctl07
 *
 * CALLS
 *	semctl(2) semget(2)
 *
 * ALGORITHM
 *	Get and manipulate a set of semaphores.
 *
 * RESTRICTIONS
 *
 * HISTORY
 *      10/03/2008 Renaud Lottiaux (Renaud.Lottiaux@kerlabs.com)
 *      - Fix concurrency issue. A statically defined key was used. Leading
 *        to conflict with other instances of the same test.
 */

#include <sys/types.h>		/* needed for test              */
#include <sys/ipc.h>		/* needed for test              */
#include <sys/sem.h>		/* needed for test              */
#include <signal.h>		/* needed for test              */
#include <errno.h>		/* needed for test              */
#include <stdio.h>		/* needed by testhead.h         */
#include <wait.h>		/* needed by testhead.h         */
#include "ipcsem.h"
#include "test.h"
#include "usctest.h"

void setup();
void cleanup();

/*
 *These globals must be defined in the test.
 */

char *TCID = "semctl07";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

int exp_enos[] = { 0 };		/* List must end with 0 */

key_t key;
int semid = -1, nsems;

/*--------------------------------------------------------------*/

int main(argc, argv)
int argc;
char *argv[];
{
	int status;
	struct semid_ds buf_ds;

	union semun {
		int val;
		struct semid_ds *buf;
		short *array;
	};

	union semun arg;

	setup();		/* temp file is now open        */
/*--------------------------------------------------------------*/

	arg.buf = &buf_ds;
	if ((status = semctl(semid, 0, IPC_STAT, arg)) == -1) {
		tst_resm(TFAIL, "semctl() failed errno = %d", errno);
		semctl(semid, 1, IPC_RMID, arg);

	}

	/*
	 * Check contents of semid_ds structure.
	 */

	if (arg.buf->sem_nsems != nsems) {
		tst_resm(TFAIL, "error: unexpected number of sems %lu",
			 arg.buf->sem_nsems);

	}
	if (arg.buf->sem_perm.uid != getuid()) {
		tst_resm(TFAIL, "error: unexpected uid %d",
			 arg.buf->sem_perm.uid);

	}
	if (arg.buf->sem_perm.gid != getgid()) {
		tst_resm(TFAIL, "error: unexpected gid %d",
			 arg.buf->sem_perm.gid);

	}
	if (arg.buf->sem_perm.cuid != getuid()) {
		tst_resm(TFAIL, "error: unexpected cuid %d",
			 arg.buf->sem_perm.cuid);

	}
	if (arg.buf->sem_perm.cgid != getgid()) {
		tst_resm(TFAIL, "error: unexpected cgid %d",
			 arg.buf->sem_perm.cgid);

	}
	if ((status = semctl(semid, 0, GETVAL, arg)) == -1) {
		tst_resm(TFAIL, "semctl(GETVAL) failed errno = %d", errno);

	}
	arg.val = 1;
	if ((status = semctl(semid, 0, SETVAL, arg)) == -1) {
		tst_resm(TFAIL, "SEMCTL(SETVAL) failed errno = %d", errno);

	}
	if ((status = semctl(semid, 0, GETVAL, arg)) == -1) {
		tst_resm(TFAIL, "semctl(GETVAL) failed errno = %d", errno);

	}
	if (status != arg.val) {
		tst_resm(TFAIL, "error: unexpected value %d", status);

	}
	if ((status = semctl(semid, 0, GETPID, arg)) == -1) {
		tst_resm(TFAIL, "semctl(GETPID) failed errno = %d", errno);

	}
	status = getpid();
	if (status == 0) {
		tst_resm(TFAIL, "error: unexpected pid %d", status);

	}
	if ((status = semctl(semid, 0, GETNCNT, arg)) == -1) {
		tst_resm(TFAIL, "semctl(GETNCNT) failed errno = %d", errno);

	}
	if (status != 0) {
		tst_resm(TFAIL, "error: unexpected semncnt %d", status);

	}
	if ((status = semctl(semid, 0, GETZCNT, arg)) == -1) {
		tst_resm(TFAIL, "semctl(GETZCNT) failed errno = %d", errno);

	}
	if (status != 0) {
		tst_resm(TFAIL, "error: unexpected semzcnt %d", status);

	}

	tst_resm(TPASS, "semctl07 ran successfully!");
/*--------------------------------------------------------------*/
/* Clean up any files created by test before exit.		*/
/*--------------------------------------------------------------*/

	cleanup();
	return (0);
}

/*--------------------------------------------------------------*/

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 *****************************************************************/
void setup()
{
	/* You will want to enable some signal handling so you can capture
	 * unexpected signals like SIGSEGV.
	 *                   */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* One cavet that hasn't been fixed yet.  TEST_PAUSE contains the code to
	 * fork the test with the -c option.  You want to make sure you do this
	 * before you create your temporary directory.
	 */
	TEST_PAUSE;

	/*
	 * Create a temporary directory and cd into it.
	 * This helps to ensure that a unique msgkey is created.
	 * See ../lib/libipc.c for more information.
	 */
	tst_tmpdir();

	/* get an IPC resource key */
	key = getipckey();
	nsems = 1;

	if ((semid = semget(key, nsems, SEM_RA | IPC_CREAT)) == -1) {
		tst_resm(TFAIL, "semget() failed errno = %d", errno);
		tst_exit();
	}
}

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 * completion or premature exit.
 ****************************************************************/
void cleanup()
{
	/* if it exists, remove the semaphore resouce */
	rm_sema(semid);

	tst_rmdir();

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}
