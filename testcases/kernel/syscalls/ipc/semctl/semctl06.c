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
 * NAME
 *	semctl06
 *
 * CALLS
 *	semctl(2) semget(2) semop(2)
 *
 * ALGORITHM
 *	Get and manipulate a set of semaphores.
 *
 * RESTRICTIONS
 *
 * WARNING
 *	If this test fail, it may be necessary to use the ipcs and ipcrm
 *	commands to remove any semaphores left in the system due to a
 *	premature exit of this test.
 *
 * HISTORY
 *      06/30/2001	Port to Linux	nsharoff@us.ibm.com
 *      10/30/2002	Port to LTP	dbarrera@us.ibm.com
 *      12/03/2008 Matthieu Fertré (Matthieu.Fertre@irisa.fr)
 *      - Fix concurrency issue. The IPC keys used for this test could
 *        conflict with keys from another task.
 */

#define DEBUG 0

#ifdef UCLINUX
#define _GNU_SOURCE
#include <stdio.h>
#endif

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include "test.h"
#include <sys/wait.h>
#include "ipcsem.h"

int local_flag = 1;

#define NREPS	500
#define NPROCS	3
#define NKIDS	5
#define NSEMS	5
#define HVAL	1000
#define LVAL	100
#define FAILED	0

void setup();
void cleanup();

static key_t keyarray[NPROCS];
static struct sembuf semops[NSEMS];
static short maxsemvals[NSEMS];
static int pidarray[NPROCS];
static int kidarray[NKIDS];
static int tid;
static int procstat;
static char *prog;
static unsigned short semvals[NSEMS];

char *TCID = "semctl06";
int TST_TOTAL = 1;

static void term(int sig);
static void dosemas(int id);
static void dotest(key_t key);

int main(int argc, char **argv)
{
	register int i, pid;
	int count, child, status, nwait;

	tst_parse_opts(argc, argv, NULL, NULL);

	prog = argv[0];
	nwait = 0;
	setup();

	tid = -1;

	for (i = 0; i < NPROCS; i++)
		keyarray[i] = getipckey();

	if ((signal(SIGTERM, term)) == SIG_ERR) {
		tst_resm(TFAIL, "\tsignal failed. errno = %d", errno);

	}

	for (i = 0; i < NPROCS; i++) {
		if ((pid = FORK_OR_VFORK()) < 0) {
			tst_resm(TFAIL,
				 "\tFork failed (may be OK if under stress)");

		}
		if (pid == 0) {
			procstat = 1;
			dotest(keyarray[i]);
			exit(0);
		}
		pidarray[i] = pid;
		nwait++;
	}

	/*
	 * Wait for children to finish.
	 */

	count = 0;
	while ((child = wait(&status)) > 0) {
		if (status) {
			tst_resm(TFAIL, "%s[%d] Test failed.  exit=0x%x", prog,
				 child, status);
			local_flag = FAILED;
		}
		++count;
	}

	/*
	 * Should have collected all children.
	 */

	if (count != nwait) {
		tst_resm(TFAIL, "\tWrong # children waited on, count = %d",
			 count);
		local_flag = FAILED;
	}

	if (local_flag != FAILED)
		tst_resm(TPASS, "semctl06 ran successfully!");
	else
		tst_resm(TFAIL, "semctl06 failed");


	cleanup();
	tst_exit();
}

static void dotest(key_t key)
{
	int id, pid, status;
	int count, child, nwait;
	short i;
	union semun get_arr;

	nwait = 0;
	srand(getpid());
	if ((id = semget(key, NSEMS, IPC_CREAT | IPC_EXCL)) < 0) {
		tst_resm(TFAIL, "\tsemget() failed errno %d", errno);
		exit(1);
	}
	tid = id;
	for (i = 0; i < NSEMS; i++) {
		do {
			maxsemvals[i] = (short) (rand() % HVAL);
		} while (maxsemvals[i] < LVAL);
		semops[i].sem_num = i;
		semops[i].sem_op = maxsemvals[i];
		semops[i].sem_flg = SEM_UNDO;
	}
	if (semop(id, semops, NSEMS) < 0) {
		tst_resm(TFAIL, "\tfirst semop() failed errno %d", errno);
		exit(1);
	}

	for (i = 0; i < NKIDS; i++) {
		if ((pid = FORK_OR_VFORK()) < 0) {
			tst_resm(TFAIL, "\tfork failed");
		}
		if (pid == 0)
			dosemas(id);
		if (pid > 0) {
			kidarray[i] = pid;
			nwait++;
		}
	}

	procstat = 2;
	/*
	 * Wait for children to finish.
	 */

	count = 0;
	while ((child = wait(&status)) > 0) {
		if (status) {
			tst_resm(TFAIL, "\t%s:dotest[%d] exited status = 0x%x",
				 prog, child, status);
			local_flag = FAILED;
		}
		++count;
	}

	/*
	 * Should have collected all children.
	 */

	if (count != nwait) {
		tst_resm(TFAIL, "\tWrong # children waited on, count = %d",
			 count);
		local_flag = FAILED;
	}

	get_arr.array = semvals;
	if (semctl(id, 0, GETALL, get_arr) < 0) {
		tst_resm(TFAIL, "\terror on GETALL");
		tst_resm(TFAIL, "\tsemctl() failed errno %d", errno);
	}

	if (DEBUG)
		tst_resm(TINFO, "\tchecking maxvals");
	for (i = 0; i < NSEMS; i++) {
		if (semvals[i] != maxsemvals[i]) {
			tst_resm(TFAIL, "\terror on i %d orig %d final %d", i,
				 semvals[i], maxsemvals[i]);
			local_flag = FAILED;
		}
	}
	if (DEBUG)
		tst_resm(TINFO, "\tmaxvals checked");

	/* 4th arg must either be missing, or must be of type 'union semun'.
	 * CANNOT just be an int, else it crashes on ppc.
	 */
	get_arr.val = 0;
	if (semctl(id, 0, IPC_RMID, get_arr) < 0) {
		tst_resm(TFAIL, "\tsemctl(IPC_RMID) failed errno %d", errno);
		local_flag = FAILED;
	}
	if (local_flag == FAILED)
		exit(1);
}

static void dosemas(int id)
{
	int i, j;

	srand(getpid());
	for (i = 0; i < NREPS; i++) {
		for (j = 0; j < NSEMS; j++) {
			semops[j].sem_num = j;
			semops[j].sem_flg = SEM_UNDO;

			do {
				semops[j].sem_op =
				    (-(short) (rand() %
							(maxsemvals[j] / 2)));
			} while (semops[j].sem_op == 0);
		}
		if (semop(id, semops, NSEMS) < 0) {
			tst_resm(TFAIL, "\tsemop1 failed errno %d", errno);
			exit(1);
		}
		for (j = 0; j < NSEMS; j++) {
			semops[j].sem_op = (-semops[j].sem_op);
		}
		if (semop(id, semops, NSEMS) < 0) {
			tst_resm(TFAIL, "\tsemop2 failed errno %d", errno);
			exit(1);
		}
	}
	exit(0);
}

static void term(int sig)
{
	int i;

	if ((signal(SIGTERM, term)) == SIG_ERR) {
		tst_resm(TFAIL, "\tsignal failed. errno %d", errno);
		exit(1);
	}
	if (procstat == 0) {
		if (DEBUG)
			tst_resm(TINFO, "\ttest killing kids");
		for (i = 0; i < NPROCS; i++) {
			if (kill(pidarray[i], SIGTERM) != 0) {
				tst_resm(TFAIL, "Kill error pid = %d :",
					 pidarray[1]);
			}
		}
		if (DEBUG)
			tst_resm(TINFO, "\ttest kids killed");
		return;
	}

	if (procstat == 1) {
		/* 4th arg must either be missing, or must be of type 'union semun'.
		 * CANNOT just be an int, else it crashes on ppc.
		 */
		union semun arg;
		arg.val = 0;
		(void)semctl(tid, 0, IPC_RMID, arg);
		exit(1);
	}

	if (tid == -1) {
		exit(1);
	}
	for (i = 0; i < NKIDS; i++) {
		if (kill(kidarray[i], SIGTERM) != 0) {
			tst_resm(TFAIL, "Kill error kid id = %d :",
				 kidarray[1]);
		}
	}
}

void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();
}

void cleanup(void)
{
	tst_rmdir();
}
