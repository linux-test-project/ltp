// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2020 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Test for a SHM_LOCK and SHM_UNLOCK.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "libnewipc.h"

#define SHM_SIZE 2048

static int shm_id = -1;

static void verify_shmlock(void)
{
	struct shmid_ds ds;

	TEST(shmctl(shm_id, SHM_LOCK, NULL));

	if (TST_RET != 0)
		tst_res(TFAIL | TTERRNO, "shmctl(%i, SHM_LOCK, NULL)", shm_id);
	else
		tst_res(TPASS, "shmctl(%i, SHM_LOCK, NULL)", shm_id);


	SAFE_SHMCTL(shm_id, IPC_STAT, &ds);

	if (ds.shm_perm.mode & SHM_LOCKED)
		tst_res(TPASS, "SMH_LOCKED bit is on in shm_perm.mode");
	else
		tst_res(TFAIL, "SHM_LOCKED bit is off in shm_perm.mode");

	TEST(shmctl(shm_id, SHM_UNLOCK, NULL));

	if (TST_RET != 0)
		tst_res(TFAIL | TTERRNO, "shmctl(%i, SHM_UNLOCK, NULL)", shm_id);
	else
		tst_res(TPASS, "shmctl(%i, SHM_UNLOCK, NULL)", shm_id);

	SAFE_SHMCTL(shm_id, IPC_STAT, &ds);

	if (ds.shm_perm.mode & SHM_LOCKED)
		tst_res(TFAIL, "SMH_LOCKED bit is on in shm_perm.mode");
	else
		tst_res(TPASS, "SHM_LOCKED bit is off in shm_perm.mode");
}

static void setup(void)
{
	shm_id = SAFE_SHMGET(IPC_PRIVATE, SHM_SIZE, IPC_CREAT | SHM_RW);
}

static void cleanup(void)
{
	if (shm_id >= 0)
		SAFE_SHMCTL(shm_id, IPC_RMID, NULL);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_shmlock,
};
