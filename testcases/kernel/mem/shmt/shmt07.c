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

/* 12/20/2002   Port to LTP     robbiew@us.ibm.com */
/* 06/30/2001   Port to Linux   nsharoff@us.ibm.com */

/*
 * NAME
 *	shmt07
 *
 * CALLS
 *	shmctl(2) shmget(2) shmat(2)
 *
 * ALGORITHM
 * Create and attach a shared memory segment, write to it
 * and then fork a child. The child verifies that the shared memory segment
 * that it inherited from the parent contains the same data that was originally
 * written to it by the parent.
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/utsname.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#define		SIZE	16*1024

/** LTP Port **/
#include "test.h"

char *TCID = "shmt07";		/* Test program identifier.    */
int TST_TOTAL = 2;		/* Total number of test cases. */
/**************/

int child();
static int rm_shm(int);

int main(void)
{
	char *cp = NULL;
	int shmid, pid, status;
	key_t key;

	key = (key_t) getpid();

/*---------------------------------------------------------*/

	errno = 0;

	if ((shmid = shmget(key, SIZE, IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		tst_brkm(TFAIL, NULL,
			 "Error: shmget: shmid = %d, errno = %d\n",
			 shmid, errno);
	}
	cp = shmat(shmid, NULL, 0);

	if (cp == (char *)-1) {
		perror("shmat");
		tst_resm(TFAIL,
			 "Error: shmat: shmid = %d, errno = %d\n",
			 shmid, errno);
		rm_shm(shmid);
		tst_exit();
	}

	*cp = '1';
	*(cp + 1) = '2';

	tst_resm(TPASS, "shmget,shmat");

/*-------------------------------------------------------*/

	pid = fork();
	switch (pid) {
	case -1:
		tst_brkm(TBROK, NULL, "fork failed");

	case 0:
		if (*cp != '1') {
			tst_resm(TFAIL, "Error: not 1\n");
		}
		if (*(cp + 1) != '2') {
			tst_resm(TFAIL, "Error: not 2\n");
		}
		tst_exit();
	}

	/* parent */
	while (wait(&status) < 0 && errno == EINTR) ;

	tst_resm(TPASS, "cp & cp+1 correct");

/*-----------------------------------------------------------*/
	rm_shm(shmid);
	tst_exit();
}

static int rm_shm(int shmid)
{
	if (shmctl(shmid, IPC_RMID, NULL) == -1) {
		perror("shmctl");
		tst_brkm(TFAIL,
			 NULL,
			 "shmctl Failed to remove: shmid = %d, errno = %d\n",
			 shmid, errno);
	}
	return (0);
}
