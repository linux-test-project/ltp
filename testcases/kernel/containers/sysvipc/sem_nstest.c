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
* In Parent Process , create semaphore with key 154326L
* Now create container by passing 1 of the below flag values..
*	clone(NONE), clone(CLONE_NEWIPC), or unshare(CLONE_NEWIPC)
* In cloned process, try to access the created semaphore
* Test PASS: If the semaphore is readable when flag is None.
* Test FAIL: If the semaphore is readable when flag is Unshare or Clone.
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

#define MY_KEY     154326L
#define UNSHARESTR "unshare"
#define CLONESTR   "clone"
#define NONESTR    "none"

char *TCID = "sem_nstest";
int TST_TOTAL = 1;
int p1[2];
int p2[2];

int check_semaphore(void *vtest)
{
	char buf[3];
	int id;

	(void) vtest;

	close(p1[1]);
	close(p2[0]);

	read(p1[0], buf, 3);
	id = semget(MY_KEY, 1, 0);
	if (id == -1)
		write(p2[1], "notfnd", 7);
	else {
		write(p2[1], "exists", 7);
		tst_resm(TINFO, "PID %d: Fetched existing semaphore..id = %d",
			 getpid(), id);
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
	int ret, use_clone = T_NONE, id;
	char *tsttype = NONESTR;
	char buf[7];

	setup();

	if (argc != 2) {
		tst_resm(TFAIL, "Usage: %s <clone| unshare| none>", argv[0]);
		tst_brkm(TFAIL, NULL, " where clone, unshare, or fork specifies"
			 " unshare method.");
	}

	/* Using PIPE's to sync between container and Parent */
	if (pipe(p1) == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}
	if (pipe(p2) == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	if (strcmp(argv[1], "clone") == 0) {
		use_clone = T_CLONE;
		tsttype = CLONESTR;
	} else if (strcmp(argv[1], "unshare") == 0) {
		use_clone = T_UNSHARE;
		tsttype = UNSHARESTR;
	}

	/* 1. Create (or fetch if existing) the binary semaphore */
	id = semget(MY_KEY, 1, IPC_CREAT | IPC_EXCL | 0666);
	if (id == -1) {
		perror("Semaphore create");
		if (errno != EEXIST) {
			perror("semget failure");
			tst_brkm(TBROK, NULL, "Semaphore creation failed");
		}
		id = semget(MY_KEY, 1, 0);
		if (id == -1) {
			perror("Semaphore create");
			tst_brkm(TBROK, NULL, "Semaphore operation failed");
		}
	}

	tst_resm(TINFO, "Semaphore namespaces Isolation test : %s", tsttype);
	/* fire off the test */
	ret =
	    do_clone_unshare_test(use_clone, CLONE_NEWIPC, check_semaphore,
				  NULL);
	if (ret < 0) {
		tst_brkm(TFAIL, NULL, "%s failed", tsttype);
	}

	close(p1[0]);
	close(p2[1]);
	write(p1[1], "go", 3);
	read(p2[0], buf, 7);

	if (strcmp(buf, "exists") == 0) {
		if (use_clone == T_NONE)
			tst_resm(TPASS, "Plain cloned process found semaphore "
				 "inside container");
		else
			tst_resm(TFAIL,
				 "%s: Container init process found semaphore",
				 tsttype);
	} else {
		if (use_clone == T_NONE)
			tst_resm(TFAIL,
				 "Plain cloned process didn't find semaphore");
		else
			tst_resm(TPASS, "%s: Container didn't find semaphore",
				 tsttype);
	}

	/* Delete the semaphore */
	id = semget(MY_KEY, 1, 0);
	semctl(id, IPC_RMID, 0);

	tst_exit();
}
