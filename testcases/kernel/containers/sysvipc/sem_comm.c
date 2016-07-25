/* Copyright (c) 2014 Red Hat, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of version 2 the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 * File: sem_comm.c
 *
 * Description:
 * 1. Clones two child processes with CLONE_NEWIPC flag, each child
 *    creates System V semaphore (sem) with the _identical_ key.
 * 2. Child1 locks the semaphore.
 * 3. Child2 locks the semaphore.
 * 4. Locking the semaphore with the identical key but from two different
 *    IPC namespaces should not interfere with each other, so if child2
 *    is able to lock the semaphore (after child1 locked it), test passes,
 *    otherwise test fails.
 */

#define _GNU_SOURCE
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include "ipcns_helper.h"
#include "test.h"
#include "safe_macros.h"
#include "lapi/semun.h"

#define TESTKEY 124426L
char *TCID	= "sem_comm";
int TST_TOTAL	= 1;

static void cleanup(void)
{
	tst_rmdir();
}

static void setup(void)
{
	tst_require_root();
	check_newipc();
	tst_tmpdir();
	TST_CHECKPOINT_INIT(tst_rmdir);
}

int chld1_sem(void *arg)
{
	int id;
	union semun su;
	struct sembuf sm;

	id = semget(TESTKEY, 1, IPC_CREAT);
	if (id == -1) {
		perror("semget");
		return 2;
	}

	su.val = 1;
	if (semctl(id, 0, SETVAL, su) == -1) {
		perror("semctl");
		semctl(id, 0, IPC_RMID);
		return 2;
	}

	/* tell child2 to continue and wait for it to create the semaphore */
	TST_SAFE_CHECKPOINT_WAKE_AND_WAIT(NULL, 0);

	sm.sem_num = 0;
	sm.sem_op = -1;
	sm.sem_flg = IPC_NOWAIT;
	if (semop(id, &sm, 1) == -1) {
		perror("semop");
		semctl(id, 0, IPC_RMID);
		return 2;
	}

	/* tell child2 to continue and wait for it to lock the semaphore */
	TST_SAFE_CHECKPOINT_WAKE_AND_WAIT(NULL, 0);

	sm.sem_op = 1;
	semop(id, &sm, 1);

	semctl(id, 0, IPC_RMID);
	return 0;
}

int chld2_sem(void *arg)
{
	int id, rval = 0;
	struct sembuf sm;
	union semun su;

	/* wait for child1 to create the semaphore */
	TST_SAFE_CHECKPOINT_WAIT(NULL, 0);

	id = semget(TESTKEY, 1, IPC_CREAT);
	if (id == -1) {
		perror("semget");
		return 2;
	}

	su.val = 1;
	if (semctl(id, 0, SETVAL, su) == -1) {
		perror("semctl");
		semctl(id, 0, IPC_RMID);
		return 2;
	}

	/* tell child1 to continue and wait for it to lock the semaphore */
	TST_SAFE_CHECKPOINT_WAKE_AND_WAIT(NULL, 0);

	sm.sem_num = 0;
	sm.sem_op = -1;
	sm.sem_flg = IPC_NOWAIT;
	if (semop(id, &sm, 1) == -1) {
		if (errno == EAGAIN) {
			rval = 1;
		} else {
			perror("semop");
			semctl(id, 0, IPC_RMID);
			return 2;
		}
	}

	/* tell child1 to continue */
	TST_SAFE_CHECKPOINT_WAKE(NULL, 0);

	sm.sem_op = 1;
	semop(id, &sm, 1);

	semctl(id, 0, IPC_RMID);
	return rval;
}

static void test(void)
{
	int status, ret = 0;

	ret = do_clone_unshare_test(T_CLONE, CLONE_NEWIPC, chld1_sem, NULL);
	if (ret == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "clone failed");

	ret = do_clone_unshare_test(T_CLONE, CLONE_NEWIPC, chld2_sem, NULL);
	if (ret == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "clone failed");


	while (wait(&status) > 0) {
		if (WIFEXITED(status) && WEXITSTATUS(status) == 1)
			ret = 1;
		if (WIFEXITED(status) && WEXITSTATUS(status) == 2)
			tst_brkm(TBROK | TERRNO, cleanup, "error in child");
		if (WIFSIGNALED(status)) {
			tst_resm(TFAIL, "child was killed with signal %s",
					tst_strsig(WTERMSIG(status)));
			return;
		}
	}

	if (ret)
		tst_resm(TFAIL, "SysV sem: communication with identical keys"
				" between namespaces");
	else
		tst_resm(TPASS, "SysV sem: communication with identical keys"
				" between namespaces");
}

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++)
		test();

	cleanup();
	tst_exit();
}
