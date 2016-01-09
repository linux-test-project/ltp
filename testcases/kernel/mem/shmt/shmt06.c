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
 *	shmt06
 *
 * CALLS
 *	shmctl(2) shmget(2) shmat(2)
 *
 * ALGORITHM
 * Parent process forks a child. Child pauses until parent has created
 * a shared memory segment, attached to it and written to it too. At that
 * time child gets the shared memory segment id, attaches to it at two
 * different addresses than the parents and verifies that their contents
 * are the same as the contents of the parent attached segment.
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/utsname.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#define		SIZE		16*1024

/** LTP Port **/
#include "test.h"

char *TCID = "shmt06";		/* Test program identifier.    */
int TST_TOTAL = 2;		/* Total number of test cases. */
/**************/

key_t key;
sigset_t set;

int child();
static int rm_shm(int);

int main(void)
{
	char *cp = NULL;
	int pid, pid1, shmid;
	int status;

	key = (key_t) getpid();

	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	sigprocmask(SIG_BLOCK, &set, NULL);

	pid = fork();
	switch (pid) {
	case -1:
		tst_brkm(TBROK, NULL, "fork failed");
	case 0:
		child();
	}

/*------------------------------------------------------*/

	if ((shmid = shmget(key, SIZE, IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		tst_resm(TFAIL, "Error: shmget: shmid = %d, errno = %d\n",
			 shmid, errno);
		/*
		 * kill the child if parent failed to do the attach
		 */
		(void)kill(pid, SIGINT);
	} else {
		cp = shmat(shmid, NULL, 0);

		if (cp == (char *)-1) {
			perror("shmat");
			tst_resm(TFAIL,
				 "Error: shmat: shmid = %d, errno = %d\n",
				 shmid, errno);

			/* kill the child if parent failed to do the attch */

			kill(pid, SIGINT);

			/* remove shared memory segment */

			rm_shm(shmid);

			tst_exit();
		}
		*cp = 'A';
		*(cp + 1) = 'B';
		*(cp + 2) = 'C';

		kill(pid, SIGUSR1);
		while ((pid1 = wait(&status)) < 0 && (errno == EINTR)) ;
		if (pid1 != pid) {
			tst_resm(TFAIL, "Waited on the wrong child");
			tst_resm(TFAIL,
				 "Error: wait_status = %d, pid1= %d\n", status,
				 pid1);
		}
	}

	tst_resm(TPASS, "shmget,shmat");

/*---------------------------------------------------------------*/

	if (shmdt(cp) < 0) {
		tst_resm(TFAIL, "shmdt");
	}

	tst_resm(TPASS, "shmdt");

/*-------------------------------------------------------------*/

	rm_shm(shmid);
	tst_exit();
}

int child(void)
{
	int shmid, chld_pid;
	char *cp;
	int sig;

	sigwait(&set, &sig);
	chld_pid = getpid();

	if ((shmid = shmget(key, SIZE, 0)) < 0) {
		perror("shmget:child process");
		tst_resm(TFAIL,
			 "Error: shmget: errno=%d, shmid=%d, child_pid=%d\n",
			 errno, shmid, chld_pid);
	} else {
		cp = shmat(shmid, NULL, 0);

		if (cp == (char *)-1) {
			perror("shmat:child process");
			tst_resm(TFAIL,
				 "Error: shmat: errno=%d, shmid=%d, child_pid=%d\n",
				 errno, shmid, chld_pid);
		} else {
			if (*cp != 'A') {
				tst_resm(TFAIL, "child: not A\n");
			}
			if (*(cp + 1) != 'B') {
				tst_resm(TFAIL, "child: not B\n");
			}
			if (*(cp + 2) != 'C') {
				tst_resm(TFAIL, "child: not C\n");
			}
			if (*(cp + 8192) != 0) {
				tst_resm(TFAIL, "child: not 0\n");
			}
		}

		/*
		 * Attach the segment to a different addresse
		 * and verify it's contents again.
		 */
		cp = shmat(shmid, NULL, 0);

		if (cp == (char *)-1) {
			perror("shmat:child process");
			tst_resm(TFAIL,
				 "Error: shmat: errno=%d, shmid=%d, child_pid=%d\n",
				 errno, shmid, chld_pid);
		} else {
			if (*cp != 'A') {
				tst_resm(TFAIL, "child: not A\n");
			}
			if (*(cp + 1) != 'B') {
				tst_resm(TFAIL, "child: not B\n");
			}
			if (*(cp + 2) != 'C') {
				tst_resm(TFAIL, "child: not C\n");
			}
			if (*(cp + 8192) != 0) {
				tst_resm(TFAIL, "child: not 0\n");
			}
		}
	}
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
