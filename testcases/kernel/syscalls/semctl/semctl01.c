// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 */
/*\
 * Test the 13 possible semctl() commands
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include "tst_safe_sysv_ipc.h"
#include "tst_test.h"
#include "lapi/sem.h"
#include "tse_newipc.h"

#define INCVAL 2
#define NEWMODE 066
#define NCHILD  5
#define SEMUN_CAST (union semun)

static int sem_id = -1;
static int sem_index;
static struct semid_ds buf;
static struct seminfo ipc_buf;
static unsigned short array[PSEMS];
static struct sembuf sops;
static int pid_arr[NCHILD];

static void kill_all_children(void)
{
	int j;

	for (j = 0; j < NCHILD; j++)
		SAFE_KILL(pid_arr[j], SIGKILL);

	for (j = 0; j < NCHILD; j++)
		SAFE_WAIT(NULL);
}

static void func_stat(void)
{
	if (buf.sem_nsems == PSEMS && buf.sem_perm.mode == (SEM_RA))
		tst_res(TPASS, "buf.sem_nsems and buf.sem_perm.mode are correct");
	else
		tst_res(TFAIL, "semaphore STAT info is incorrect");
}

static void set_setup(void)
{
	buf.sem_perm.mode = SEM_RA | NEWMODE;
}

static void func_set(void)
{
	SAFE_SEMCTL(sem_id, 0, IPC_STAT, (union semun)&buf);

	if (buf.sem_perm.mode == (SEM_RA | NEWMODE))
		tst_res(TPASS, "buf.sem_perm.mode is correct");
	else
		tst_res(TFAIL, "semaphore mode info is incorrect");
}

static void func_gall(void)
{
	int i;

	for (i = 0; i < PSEMS; i++) {
		if (array[i] != 0) {
			tst_res(TFAIL, "semaphore %d has unexpected value", i);
			return;
		}
	}
	tst_res(TPASS, "semaphores have expected values");
}

static void child_cnt(void)
{
	sops.sem_num = 4;
	sops.sem_flg = 0;

	/*
	 * Do a semop that will cause the child to sleep.
	 * The child process will be killed in the func_ncnt
	 * routine which should cause an error to be return
	 * by the semop() call.
	 */
	if (semop(sem_id, &sops, 1) != -1)
		tst_brk(TBROK, "semop succeeded - cnt_setup");
}

static void cnt_setup(int opval)
{
	int pid, i;

	sops.sem_num = 4;
	sops.sem_flg = 0;
	/*
	 * if seting up for GETZCNT, the semaphore value needs to be positive
	 */
	if (opval == 0) {
		sops.sem_op = 1;
		SAFE_SEMOP(sem_id, &sops, 1);
	}

	sops.sem_op = opval;
	for (i = 0; i < NCHILD; i++) {
		pid = SAFE_FORK();
		if (pid == 0) {
			child_cnt();
		} else {
			TST_PROCESS_STATE_WAIT(pid, 'S', 0);
			pid_arr[i] = pid;
		}
	}
}

static void func_cnt(int rval)
{
	if (rval == NCHILD)
		tst_res(TPASS, "number of sleeping processes is correct");
	else
		tst_res(TFAIL, "number of sleeping processes is not correct");
}

static void child_pid(void)
{
	sops.sem_num = 2;
	sops.sem_op = 1;
	sops.sem_flg = 0;
	/*
	 * Do a semop that will increment the semaphore.
	 */
	SAFE_SEMOP(sem_id, &sops, 1);
	exit(0);
}

static void pid_setup(void)
{
	int pid;

	pid = SAFE_FORK();
	if (pid == 0) {
		child_pid();
	} else {
		pid_arr[2] = pid;
		TST_PROCESS_STATE_WAIT(pid, 'Z', 0);
	}
}

static void func_pid(int rval)
{
	if (rval == pid_arr[2])
		tst_res(TPASS, "last pid value is correct");
	else
		tst_res(TFAIL, "last pid value is not correct");
}

static void func_gval(int rval)
{
	/*
	 * This is a simple test.  The semaphore value should be equal
	 * to ONE as it was set in the last test (GETPID).
	 */
	if (rval == 1)
		tst_res(TPASS, "semaphore value is correct");
	else
		tst_res(TFAIL, "semaphore value is not correct");
}

static void sall_setup(void)
{
	int i;

	for (i = 0; i < PSEMS; i++) {
		array[i] = 3;
	}
}

static void func_sall(void)
{
	int i;
	unsigned short rarray[PSEMS];

	SAFE_SEMCTL(sem_id, 0, GETALL, (union semun)rarray);
	for (i = 0; i < PSEMS; i++) {
		if (array[i] != rarray[i]) {
			tst_res(TFAIL, "semaphore values are not correct");
			return;
		}
	}

	tst_res(TPASS, "semaphore values are correct");
}

static void func_sval(void)
{
	int semv = SAFE_SEMCTL(sem_id, 4, GETVAL);

	if (semv != INCVAL)
		tst_res(TFAIL, "semaphore value is not what was set");
	else
		tst_res(TPASS, "semaphore value is correct");
}

static void func_rmid(void)
{
	TST_EXP_FAIL(semop(sem_id, &sops, 1), EINVAL, "semaphore appears to be removed");
	sem_id = -1;
}

static void func_iinfo(int hidx)
{
	if (hidx >= 0) {
		sem_index = hidx;
		tst_res(TPASS, "the highest index is correct");
	} else {
		sem_index = 0;
		tst_res(TFAIL, "the highest index is incorrect");
	}
}

static void func_sinfo(void)
{
	if (ipc_buf.semusz < 1)
		tst_res(TFAIL, "number of semaphore sets is incorrect");
	else
		tst_res(TPASS, "number of semaphore sets is correct");
}

static void func_sstat(int semidx)
{
	if (semidx >= 0)
		tst_res(TPASS, "id of the semaphore set is correct");
	else
		tst_res(TFAIL, "id of the semaphore set is incorrect");
}

static struct tcases {
	int *semid;
	int semnum;
	int cmd;
	void (*func_test) ();
	union semun arg;
	void (*func_setup) ();
} tests[] = {
	{&sem_id, 0, IPC_STAT, func_stat, SEMUN_CAST & buf, NULL},
	{&sem_id, 0, IPC_SET, func_set, SEMUN_CAST & buf, set_setup},
	{&sem_id, 0, GETALL, func_gall, SEMUN_CAST array, NULL},
	{&sem_id, 4, GETNCNT, func_cnt, SEMUN_CAST & buf, cnt_setup},
	{&sem_id, 2, GETPID, func_pid, SEMUN_CAST & buf, pid_setup},
	{&sem_id, 2, GETVAL, func_gval, SEMUN_CAST & buf, NULL},
	{&sem_id, 4, GETZCNT, func_cnt, SEMUN_CAST & buf, cnt_setup},
	{&sem_id, 0, SETALL, func_sall, SEMUN_CAST array, sall_setup},
	{&sem_id, 4, SETVAL, func_sval, SEMUN_CAST INCVAL, NULL},
	{&sem_id, 0, IPC_INFO, func_iinfo, SEMUN_CAST & ipc_buf, NULL},
	{&sem_id, 0, SEM_INFO, func_sinfo, SEMUN_CAST & ipc_buf, NULL},
	{&sem_index, 0, SEM_STAT, func_sstat, SEMUN_CAST & buf, NULL},
	{&sem_id, 0, IPC_RMID, func_rmid, SEMUN_CAST & buf, NULL},
};

static void verify_semctl(unsigned int n)
{
	struct tcases *tc = &tests[n];
	int rval;

	if (sem_id == -1)
		sem_id = SAFE_SEMGET(IPC_PRIVATE, PSEMS, IPC_CREAT | IPC_EXCL | SEM_RA);
	if (tc->func_setup) {
		switch (tc->cmd) {
		case GETNCNT:
			tc->func_setup(-1);
			break;
		case GETZCNT:
			tc->func_setup(0);
			break;
		default:
			tc->func_setup();
			break;
		}
	}

	rval = SAFE_SEMCTL(*(tc->semid), tc->semnum, tc->cmd, tc->arg);
	switch (tc->cmd) {
	case GETNCNT:
	case GETZCNT:
	case GETPID:
	case GETVAL:
	case IPC_INFO:
	case SEM_STAT:
		tc->func_test(rval);
		break;
	default:
		tc->func_test();
		break;
	}

	if (tc->cmd == GETNCNT || tc->cmd == GETZCNT)
		kill_all_children();
}

static void cleanup(void)
{
	if (sem_id >= 0)
		SAFE_SEMCTL(sem_id, 0, IPC_RMID);
}

static struct tst_test test = {
	.cleanup = cleanup,
	.test = verify_semctl,
	.tcnt = ARRAY_SIZE(tests),
	.forks_child = 1,
};
