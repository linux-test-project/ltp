/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
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

/*
 *  FILE        : sem01.c
 *  DESCRIPTION : Creates a semaphore and two processes.  The processes
 *                each go through a loop where they semdown, delay for a
 *                random amount of time, and semup, so they will almost
 *                always be fighting for control of the semaphore.
 *  HISTORY:
 *    01/15/2001 Paul Larson (plars@us.ibm.com)
 *      -written
 *    11/09/2001 Manoj Iyer (manjo@ausin.ibm.com)
 *    Modified.
 *    - Removed compiler warnings.
 *      added exit to the end of function main()
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include "lapi/sem.h"

int verbose = 0;
int loops = 100;
int errors = 0;

int semup(int semid)
{
	struct sembuf semops;
	semops.sem_num = 0;
	semops.sem_op = 1;
	semops.sem_flg = SEM_UNDO;
	if (semop(semid, &semops, 1) == -1) {
		perror("semup");
		errors++;
		return 1;
	}
	return 0;
}

int semdown(int semid)
{
	struct sembuf semops;
	semops.sem_num = 0;
	semops.sem_op = -1;
	semops.sem_flg = SEM_UNDO;
	if (semop(semid, &semops, 1) == -1) {
		perror("semdown");
		errors++;
		return 1;
	}
	return 0;
}

void delayloop()
{
	int delay;
	delay = 1 + ((100.0 * rand()) / RAND_MAX);
	if (verbose)
		printf("in delay function for %d microseconds\n", delay);
	usleep(delay);
}

void mainloop(int semid)
{
	int i;
	for (i = 0; i < loops; i++) {
		if (semdown(semid)) {
			printf("semdown failed\n");
		}
		if (verbose)
			printf("sem is down\n");
		delayloop();
		if (semup(semid)) {
			printf("semup failed\n");
		}
		if (verbose)
			printf("sem is up\n");
	}
}

int main(int argc, char *argv[])
{
	int semid, opt;
	union semun semunion;
	extern char *optarg;
	pid_t pid;
	int chstat;

	while ((opt = getopt(argc, argv, "l:vh")) != EOF) {
		switch ((char)opt) {
		case 'l':
			loops = atoi(optarg);
			break;
		case 'v':
			verbose = 1;
			break;
		case 'h':
		default:
			printf("Usage: -l loops [-v]\n");
			exit(1);
		}
	}

	/* set up the semaphore */
	if ((semid = semget((key_t) 9142, 1, 0666 | IPC_CREAT)) < 0) {
		printf("error in semget()\n");
		exit(-1);
	}
	semunion.val = 1;
	if (semctl(semid, 0, SETVAL, semunion) == -1) {
		printf("error in semctl\n");
	}

	if ((pid = fork()) < 0) {
		printf("fork error\n");
		exit(-1);
	}
	if (pid) {
		/* parent */
		srand(pid);
		mainloop(semid);
		waitpid(pid, &chstat, 0);
		if (!WIFEXITED(chstat)) {
			printf("child exited with status\n");
			exit(-1);
		}
		if (semctl(semid, 0, IPC_RMID, semunion) == -1) {
			printf("error in semctl\n");
		}
		if (errors) {
			printf("FAIL: there were %d errors\n", errors);
		} else {
			printf("PASS: error count is 0\n");
		}
		exit(errors);
	} else {
		/* child */
		mainloop(semid);
	}
	exit(0);
}
