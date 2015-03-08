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
 *		 shmt05
 *
 * CALLS
 *		 shmctl(2) shmget(2) shmat(2)
 *
 * ALGORITHM
 * Create two shared memory segments and attach them to the same process
 * at two different addresses. The addresses DO BUMP into each other.
 * The second attach should Fail.
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/utsname.h>
#include <errno.h>
#include <time.h>

/** LTP Port **/
#include "test.h"

char *TCID = "shmt05";		/* Test program identifier.    */
int TST_TOTAL = 2;		/* Total number of test cases. */
/**************/

key_t key[2];

#define		 SIZE		 (2*SHMLBA)

static int rm_shm(int);

int main(void)
{
	int shmid, shmid1;
	char *cp, *cp1;

	srand48((getpid() << 16) + (unsigned)time(NULL));

	key[0] = (key_t) lrand48();
	key[1] = (key_t) lrand48();

	cp = NULL;
	cp1 = NULL;

/*--------------------------------------------------------*/

	if ((shmid = shmget(key[0], SIZE, IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		tst_resm(TFAIL,
			 "Error: shmget: shmid = %d, errno = %d\n",
			 shmid, errno);
	} else {
		cp = shmat(shmid, NULL, 0);

		if (cp == (char *)-1) {
			tst_resm(TFAIL, "shmat");
			rm_shm(shmid);
		}
	}

	tst_resm(TPASS, "shmget & shmat");

/*--------------------------------------------------------*/

	if ((shmid1 = shmget(key[1], SIZE, IPC_CREAT | 0666)) < 0) {
		perror("shmget2");
		tst_resm(TFAIL,
			 "Error: shmget: shmid1 = %d, errno = %d\n",
			 shmid1, errno);
	} else {
		cp1 = shmat(shmid1, cp + (SIZE / 2), 0);
		if (cp1 != (char *)-1) {
			perror("shmat");
			tst_resm(TFAIL,
				 "Error: shmat: shmid1 = %d, addr= %p, errno = %d\n",
				 shmid1, cp1, errno);
		} else {
			tst_resm(TPASS, "2nd shmget & shmat");
		}
	}

/*------------------------------------------------------*/

	rm_shm(shmid);
	rm_shm(shmid1);

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
