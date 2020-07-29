// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * E2BIG   - nsops is greater than max number of operations allowed per syscall
 * EACCESS - calling process does not have permission to alter semaphore
 * EFAULT  - invalid address passed for sops
 * EINVAL  - nonpositive nsops
 * EINVAL  - negative semid
 * ERANGE  - semop + semval > semvmx a maximal semaphore value
 * EFBIG   - sem_num less than zero
 * EFBIG   - sem_num > number of semaphores in a set
 * EAGAIN  - semop = 0 for non-zero semaphore and IPC_NOWAIT passed in flags
 * EAGAIN  - semop = -1 for zero semaphore and IPC_NOWAIT passed in flags
 *
 * Copyright (c) International Business Machines  Corp., 2001
 *	03/2001 - Written by Wayne Boyer
 *	10/03/2008 Renaud Lottiaux (Renaud.Lottiaux@kerlabs.com)
 */

#define _GNU_SOURCE
#include <pwd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "tst_test.h"
#include "libnewipc.h"
#include "lapi/semun.h"

static int valid_sem_id = -1;
static int noperm_sem_id = -1;
static int bad_sem_id = -1;
static short sem_op_max, sem_op_1 = 1, sem_op_negative = -1, sem_op_zero = 0;
static struct sembuf *faulty_buf;

#define NSOPS	1
#define	BIGOPS	1024

static struct test_case_t {
	int *semid;
	struct sembuf **buf;
	short *sem_op;
	unsigned short ctl_sem_num;
	unsigned short sem_num;
	short sem_flg;
	unsigned t_ops;
	int arr_val;
	int error;
} tc[] = {
	{&valid_sem_id, NULL, &sem_op_1, 0, 0, 0, BIGOPS, 1, E2BIG},
	{&noperm_sem_id, NULL, &sem_op_1, 0, 0, 0, NSOPS, 1, EACCES},
	{&valid_sem_id, &faulty_buf, &sem_op_1, 0, 0, 0, NSOPS, 1, EFAULT},
	{&valid_sem_id, NULL, &sem_op_1, 0, 0, 0, 0, 1, EINVAL},
	{&bad_sem_id,   NULL, &sem_op_1, 0, 0, 0, NSOPS, 1, EINVAL},
	{&valid_sem_id, NULL, &sem_op_max, 0, 0, 0, 1, 1, ERANGE},
	{&valid_sem_id, NULL, &sem_op_1, 0, -1, SEM_UNDO, 1, 1, EFBIG},
	{&valid_sem_id, NULL, &sem_op_1, 0, PSEMS + 1, SEM_UNDO, 1, 1, EFBIG},
	{&valid_sem_id, NULL, &sem_op_zero, 2, 2, IPC_NOWAIT, 1, 1, EAGAIN},
	{&valid_sem_id, NULL, &sem_op_negative, 2, 2, IPC_NOWAIT, 1, 0, EAGAIN}
};

static void setup(void)
{
	struct passwd *ltpuser;
	key_t semkey;
	union semun arr;
	struct seminfo ipc_buf;

	ltpuser = SAFE_GETPWNAM("nobody");
	SAFE_SETUID(ltpuser->pw_uid);

	semkey = GETIPCKEY();

	valid_sem_id = semget(semkey, PSEMS, IPC_CREAT | IPC_EXCL | SEM_RA);
	if (valid_sem_id == -1)
		tst_brk(TBROK | TERRNO, "couldn't create semaphore in setup");

	semkey = GETIPCKEY();

	noperm_sem_id = semget(semkey, PSEMS, IPC_CREAT | IPC_EXCL);
	if (noperm_sem_id == -1)
		tst_brk(TBROK | TERRNO, "couldn't create semaphore in setup");

	arr.__buf = &ipc_buf;
	if (semctl(valid_sem_id, 0, IPC_INFO, arr) == -1)
		tst_brk(TBROK | TERRNO, "semctl() IPC_INFO failed");

	sem_op_max = ipc_buf.semvmx;
	faulty_buf = tst_get_bad_addr(NULL);
}

static void run(unsigned int i)
{
	union semun arr = {.val = tc[i].arr_val};
	struct sembuf buf = {
		.sem_op = *tc[i].sem_op,
		.sem_flg = tc[i].sem_flg,
		.sem_num = tc[i].sem_num,
	};
	struct sembuf *ptr = &buf;

	if (*tc[i].semid == valid_sem_id) {
		if (semctl(valid_sem_id, tc[i].ctl_sem_num, SETVAL, arr) == -1)
			tst_brk(TBROK | TERRNO, "semctl() SETVAL failed");
	}

	if (tc[i].buf)
		ptr = *tc[i].buf;

	TEST(semop(*(tc[i].semid), ptr, tc[i].t_ops));

	if (TST_RET != -1) {
		tst_res(TFAIL | TTERRNO, "call succeeded unexpectedly");
		return;
	}

	if (TST_ERR == tc[i].error) {
		tst_res(TPASS | TTERRNO, "semop failed as expected");
	} else {
		tst_res(TFAIL | TTERRNO,
		        "semop failed unexpectedly; expected: %s",
		        tst_strerrno(tc[i].error));
	}
}

static void cleanup(void)
{
	if (valid_sem_id != -1) {
		if (semctl(valid_sem_id, 0, IPC_RMID) == -1)
			tst_res(TWARN, "semaphore deletion failed.");
	}

	if (noperm_sem_id != -1) {
		if (semctl(noperm_sem_id, 0, IPC_RMID) == -1)
			tst_res(TWARN, "semaphore deletion failed.");
	}
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tc),
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.needs_root = 1,
};
