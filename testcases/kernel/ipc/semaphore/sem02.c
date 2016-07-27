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

/*
 *  FILE        : sem02.c
 *
 *  DESCRIPTION : The application creates several threads using pthread_create().
 *  One thread performs a semop() with the SEM_UNDO flag set. The change in
 *  sempaphore value performed by that semop should be "undone" only when the
 *  last pthread exits.
 *
 *  EXPECTED OUTPUT:
 *  Waiter, pid = <pid#>
 *  Poster, pid = <pid#>, posting
 *  Poster posted
 *  Poster exiting
 *  Waiter waiting, pid = <pid#>
 *  Waiter done waiting
 *
 *  HISTORY:
 *    written by Dave Olien (oliend@us.ibm.com)
 *    03/06/2002 Robbie Williamson (robbiew@us.ibm.com)
 *      -ported
 *    07/04/2003 Paul Larson (plars@linuxtestproject.org)
 *      -ported to LTP
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sem.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include "lapi/semun.h"
#include "test.h"

#define KEY IPC_PRIVATE

#define NUMTHREADS 2

void *retval[NUMTHREADS];
void *waiter(void *);
void *poster(void *);
void cleanup(void);

char *TCID = "sem02";
int TST_TOTAL = 1;

struct sembuf Psembuf = { 0, -1, SEM_UNDO };
struct sembuf Vsembuf = { 0, 1, SEM_UNDO };

int sem_id;
int err_ret;			/* This is used to determine PASS/FAIL status */
int main(int argc, char **argv)
{
	int i, rc;
	union semun semunion;

	pthread_t pt[NUMTHREADS];
	pthread_attr_t attr;

	tst_parse_opts(argc, argv, NULL, NULL);
	/* Create the semaphore set */
	sem_id = semget(KEY, 1, 0666 | IPC_CREAT);
	if (sem_id < 0) {
		printf("semget failed, errno = %d\n", errno);
		exit(1);
	}
	/* initialize data  structure associated to the semaphore */
	semunion.val = 1;
	semctl(sem_id, 0, SETVAL, semunion);

	/* setup the attributes of the thread        */
	/* set the scope to be system to make sure the threads compete on a  */
	/* global scale for cpu   */
	pthread_attr_init(&attr);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

	err_ret = 1;		/* Set initial error value to 1 */
	/* Create the threads */
	for (i = 0; i < NUMTHREADS; i++) {
		if (i == 0)
			rc = pthread_create(&pt[i], &attr, waiter, retval[i]);
		else
			rc = pthread_create(&pt[i], &attr, poster, retval[i]);
	}

	/* Sleep long enough to see that the other threads do what they are supposed to do */
	sleep(20);
	semunion.val = 1;
	semctl(sem_id, 0, IPC_RMID, semunion);
	if (err_ret == 1)
		tst_resm(TFAIL, "failed");
	else
		tst_resm(TPASS, "passed");
	cleanup();

	tst_exit();
}

/* This thread sleeps 10 seconds then waits on the semaphore.  As long
   as someone has posted on the semaphore, and no undo has taken
   place, the semop should complete and we'll print "Waiter done
   waiting." */
void *waiter(void *foo)
{
	int pid;
	pid = getpid();

	tst_resm(TINFO, "Waiter, pid = %d", pid);
	sleep(10);

	tst_resm(TINFO, "Waiter waiting, pid = %d", pid);
	semop(sem_id, &Psembuf, 1);
	tst_resm(TINFO, "Waiter done waiting");
	err_ret = 0;		/* If the message above is displayed, the test is a PASS */
	pthread_exit(0);
}

/* This thread immediately posts on the semaphore and then immediately
   exits.  If the *thread* exits, the undo should not happen, and the
   waiter thread which will start waiting on it in 10 seconds, should
   still get it.   */
void *poster(void *foo)
{
	int pid;

	pid = getpid();
	tst_resm(TINFO, "Poster, pid = %d, posting", pid);
	semop(sem_id, &Vsembuf, 1);
	tst_resm(TINFO, "Poster posted");
	tst_resm(TINFO, "Poster exiting");

	pthread_exit(0);
}

void cleanup(void)
{
}
