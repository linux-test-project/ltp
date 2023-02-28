// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 */

/*\
 * [Description]
 *
 * This case checks that semget() correclty creates a semaphore set.
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include "lapi/sem.h"
#include "tst_test.h"
#include "libnewipc.h"
#include "tst_safe_sysv_ipc.h"

static int sem_id = -1, sem_key = -1;

static void check_functionality(void)
{
	struct semid_ds semary;
	union semun un_arg;

	un_arg.buf = &semary;
	SAFE_SEMCTL(sem_id, 0, IPC_STAT, un_arg);
	TST_EXP_EQ_LI(un_arg.buf->sem_nsems, PSEMS);
	TST_EXP_EQ_LI(un_arg.buf->sem_perm.cuid, geteuid());

	tst_res(TPASS, "basic semaphore values are okay");
}

static void verify_semget(void)
{
	TEST(semget(sem_key, PSEMS, IPC_CREAT | IPC_EXCL | SEM_RA));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "semget() failed");
		return;
	}

	sem_id = TST_RET;
	check_functionality();

	SAFE_SEMCTL(sem_id, PSEMS, IPC_RMID);
}

static void setup(void)
{
	sem_key = GETIPCKEY();
}

static void cleanup(void)
{
	if (sem_id != -1)
		SAFE_SEMCTL(sem_id, PSEMS, IPC_RMID);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_semget,
};
