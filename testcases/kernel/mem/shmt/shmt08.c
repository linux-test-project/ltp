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
 *	shmt08
 *
 * CALLS
 *	shmctl(2) shmget(2) shmat(2) shmdt(2)
 *
 * ALGORITHM
 * Create a shared memory segment. Attach it twice at an address
 * that is provided by the system.  Detach the previously attached
 * segments from the process.
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

#define K_1  1024

/** LTP Port **/
#include "test.h"

char *TCID = "shmt08";		/* Test program identifier.    */
int TST_TOTAL = 2;		/* Total number of test cases. */
/**************/

key_t key;

static int rm_shm(int);

int main(void)
{
	char *cp = NULL, *cp1 = NULL;
	int shmid;

	key = (key_t) getpid();
	errno = 0;
/*-------------------------------------------------------*/

	if ((shmid = shmget(key, 24 * K_1, IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		tst_brkm(TFAIL, NULL,
			 "Error: shmget: shmid = %d, errno = %d\n",
			 shmid, errno);
	}

	cp = shmat(shmid, NULL, 0);
	if (cp == (char *)-1) {
		tst_resm(TFAIL, "shmat1 Failed");
		rm_shm(shmid);
		tst_exit();
	}

	cp1 = shmat(shmid, NULL, 0);
	if (cp1 == (char *)-1) {
		perror("shmat2");
		rm_shm(shmid);
		tst_exit();
	}

	tst_resm(TPASS, "shmget,shmat");

/*--------------------------------------------------------*/

	if (shmdt(cp) < 0) {
		perror("shmdt2");
		tst_resm(TFAIL, "shmdt:cp");
	}

	if (shmdt(cp1) < 0) {
		perror("shmdt1");
		tst_resm(TFAIL, "shmdt:cp1");
	}

	tst_resm(TPASS, "shmdt");

/*---------------------------------------------------------*/
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
