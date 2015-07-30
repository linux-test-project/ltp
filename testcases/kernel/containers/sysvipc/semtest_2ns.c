/* *************************************************************************
* Copyright (c) International Business Machines Corp., 2009
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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*
* Author: Veerendra C <vechandr@in.ibm.com>
*
* Test Assertion:
* This testcase verifies the semaphore isoloation in 2 diff containers.
* It tries to create/access a semaphore created with the same KEY.
*
* Description:
* Create 2 'containers' with the below flag value
*   Flag = clone, clone(CLONE_NEWIPC), or unshare(CLONE_NEWIPC)
* In Cont1, create semaphore with key 124326L
* In Cont2, try to access the semaphore created in Cont1.
* PASS :
*		If flag = None and the semaphore is accessible in Cont2.
*		If flag = unshare/clone and the semaphore is not accessible in Cont2.
*		If semaphore is not accessible in Cont2, creates new semaphore with
*		the same key to double check isloation in IPCNS.
*
* FAIL :
*		If flag = none and the semaphore is not accessible.
*		If flag = unshare/clone and semaphore is accessible in Cont2.
*		If the new semaphore creation Fails.
***************************************************************************/

#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <libclone.h>
#include "test.h"
#include "ipcns_helper.h"

#define MY_KEY     124326L
#define UNSHARESTR "unshare"
#define CLONESTR   "clone"
#define NONESTR    "none"

char *TCID = "semtest_2ns";
int TST_TOTAL = 1;
int p1[2];
int p2[2];
static struct sembuf semop_lock[2] = {
	/* sem_num, sem_op, flag */
	{0, 0, 0},		/* wait for sem#0 to become 0 */
	{0, 1, SEM_UNDO}	/* then increment sem#0 by 1 */
};

static struct sembuf semop_unlock[1] = {
	/* sem_num, sem_op, flag */
	{0, -1, (IPC_NOWAIT | SEM_UNDO)}	/* decrement sem#0 by 1 (sets it to 0) */
};

/*
 * sem_lock() - Locks the semaphore for crit-sec updation, and unlocks it later
 */
void sem_lock(int id)
{
	/* Checking the semlock and simulating as if the crit-sec is updated */
	if (semop(id, &semop_lock[0], 2) < 0) {
		perror("sem lock error");
		tst_brkm(TBROK, NULL, "semop failed");
	}
	tst_resm(TINFO, "Sem1: File locked, Critical section is updated...");
	sleep(2);
	if (semop(id, &semop_unlock[0], 1) < 0) {
		perror("sem unlock error");
		tst_brkm(TBROK, NULL, "semop failed");
	}
}

/*
 * check_sem1 -  does not read -- it writes to check_sem2() when it's done.
 */
int check_sem1(void *vtest)
{
	int id1;

	(void) vtest;

	close(p1[0]);
	/* 1. Create (or fetch if existing) the binary semaphore */
	id1 = semget(MY_KEY, 1, IPC_CREAT | IPC_EXCL | 0666);
	if (id1 == -1) {
		perror("Semaphore create");
		if (errno != EEXIST) {
			perror("semget failure");
			tst_brkm(TBROK, NULL, "semget failure");
		}
		id1 = semget(MY_KEY, 1, 0);
		if (id1 == -1) {
			perror("Semaphore create");
			tst_brkm(TBROK, NULL, "semget failure");
		}
	}

	write(p1[1], "go", 3);
	tst_resm(TINFO, "Cont1: Able to create semaphore");
	tst_exit();
}

/*
 * check_sem2() reads from check_sem1() and writes to main() when it's done.
 */

int check_sem2(void *vtest)
{
	char buf[3];
	int id2;

	(void) vtest;

	close(p1[1]);
	close(p2[0]);
	read(p1[0], buf, 3);

	id2 = semget(MY_KEY, 1, 0);
	if (id2 != -1) {
		sem_lock(id2);
		write(p2[1], "exists", 7);
	} else {
		/* Trying to create a new semaphore, if semaphore is not existing */
		id2 = semget(MY_KEY, 1, IPC_CREAT | IPC_EXCL | 0666);
		if (id2 == -1) {
			perror("Semaphore create");
			if (errno != EEXIST) {
				perror("semget failure");
				tst_resm(TBROK, "semget failure");
			}
		} else
			tst_resm(TINFO,
				 "Cont2: Able to create semaphore with sameKey");
		/* Passing the pipe Not-found mesg */
		write(p2[1], "notfnd", 7);
	}

	tst_exit();
}

static void setup(void)
{
	tst_require_root();
	check_newipc();
}

int main(int argc, char *argv[])
{
	int ret, id, use_clone = T_NONE;
	char *tsttype = NONESTR;
	char buf[7];

	setup();

	if (argc != 2) {
		tst_resm(TINFO, "Usage: %s <clone| unshare| none>", argv[0]);
		tst_resm(TINFO, " where clone, unshare, or fork specifies"
			 " unshare method.");
		tst_exit();
	}

	/* Using PIPE's to sync between container and Parent */
	if (pipe(p1) == -1) {
		perror("pipe1");
		tst_exit();
	}
	if (pipe(p2) == -1) {
		perror("pipe2");
		tst_exit();
	}

	if (strcmp(argv[1], "clone") == 0) {
		use_clone = T_CLONE;
		tsttype = CLONESTR;
	} else if (strcmp(argv[1], "unshare") == 0) {
		use_clone = T_UNSHARE;
		tsttype = UNSHARESTR;
	}

	tst_resm(TINFO, "Semaphore Namespaces Test : %s", tsttype);

	/* Create 2 containers */
	ret = do_clone_unshare_test(use_clone, CLONE_NEWIPC, check_sem1, NULL);
	if (ret < 0) {
		tst_brkm(TFAIL, NULL, "clone/unshare failed");
	}

	ret = do_clone_unshare_test(use_clone, CLONE_NEWIPC, check_sem2, NULL);
	if (ret < 0) {
		tst_brkm(TFAIL, NULL, "clone/unshare failed");
	}
	close(p2[1]);
	read(p2[0], buf, 7);

	if (strcmp(buf, "exists") == 0)
		if (use_clone == T_NONE)
			tst_resm(TPASS,
				 "Plain cloned process able to access the semaphore "
				 "created");
		else
			tst_resm(TFAIL,
				 "%s : In namespace2 found the semaphore "
				 "created in Namespace1", tsttype);
	else if (use_clone == T_NONE)
		tst_resm(TFAIL, "Plain cloned process didn't find semaphore");
	else
		tst_resm(TPASS,
			 "%s : In namespace2 unable to access the semaphore "
			 "created in Namespace1", tsttype);

	/* Delete the semaphore */
	id = semget(MY_KEY, 1, 0);
	semctl(id, IPC_RMID, 0);
	tst_exit();
}
