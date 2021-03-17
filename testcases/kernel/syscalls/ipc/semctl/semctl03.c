// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 */
/*\
 * [Description]
 *
 * Test for semctl() EINVAL and EFAULT errors
 */

#include "tst_safe_sysv_ipc.h"
#include "tst_test.h"
#include "lapi/sem.h"
#include "libnewipc.h"

static int sem_id = -1;
static int bad_id = -1;

static struct semid_ds sem_ds;
static union semun sem_un = {.buf = &sem_ds};
static void *semds_ptr = &sem_un;
static void *bad_ptr;

static struct tcases {
	int *sem_id;
	int ipc_cmd;
	void **buf;
	int error;
	char *message;
} tests[] = {
	{&sem_id, -1, &semds_ptr, EINVAL, "invalid IPC command"},
	{&bad_id, IPC_STAT, &semds_ptr, EINVAL, "invalid sem id"},
	{&sem_id, GETALL, &bad_ptr, EFAULT, "invalid union arg"},
	{&sem_id, IPC_SET, &bad_ptr, EFAULT, "invalid union arg"}
};

static void verify_semctl(unsigned int n)
{
	struct tcases *tc = &tests[n];

	TST_EXP_FAIL(semctl(*(tc->sem_id), 0, tc->ipc_cmd, *(tc->buf)),
		     tc->error, "semctl() with %s", tc->message);
}

static void setup(void)
{
	static key_t semkey;

	semkey = GETIPCKEY();

	sem_id = SAFE_SEMGET(semkey, PSEMS, IPC_CREAT | IPC_EXCL | SEM_RA);

	bad_ptr = tst_get_bad_addr(NULL);
}

void cleanup(void)
{
	if (sem_id != -1)
		SAFE_SEMCTL(sem_id, 0, IPC_RMID);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_semctl,
	.tcnt = ARRAY_SIZE(tests),
};
