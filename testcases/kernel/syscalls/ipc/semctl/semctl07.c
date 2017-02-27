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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include "ipcsem.h"
#include "test.h"

void setup(void);
void cleanup(void);

char *TCID = "semctl07";
int TST_TOTAL = 1;

key_t key;
int semid = -1, nsems;

int main(int argc, char *argv[])
{
	int status;
	struct semid_ds buf_ds;
	union semun arg;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

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

	cleanup();
	tst_exit();
}

void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	/* get an IPC resource key */
	key = getipckey();
	nsems = 1;

	if ((semid = semget(key, nsems, SEM_RA | IPC_CREAT)) == -1) {
		tst_brkm(TFAIL, NULL, "semget() failed errno = %d", errno);
	}
}

void cleanup(void)
{
	rm_sema(semid);
	tst_rmdir();
}
