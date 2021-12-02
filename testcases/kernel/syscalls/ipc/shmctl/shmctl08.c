// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2020 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * [Description]
 *
 * Test for a SHM_SET.
 *
 * The test clears the group and others bits from the shm_perm.mode and checks
 * the result as well as if the ctime was updated correctly.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "libnewipc.h"

#define SHM_SIZE 2048

static int shm_id = -1;

static int test_ipc_set(struct shmid_ds *ds)
{
	TEST(shmctl(shm_id, IPC_SET, ds));

	if (TST_RET != 0) {
		tst_res(TFAIL, "shmctl(%i, IPC_SET, ...)", shm_id);
		return 1;
	}

	tst_res(TPASS, "shmctl(%i, IPC_SET, {shm_perm.mode=%04o})",
		shm_id, ds->shm_perm.mode);
	return 0;
}

static void check_mode(struct shmid_ds *ds, short exp_mode)
{
	if (ds->shm_perm.mode == exp_mode) {
		tst_res(TPASS, "shm_perm.mode=%04o", exp_mode);
		return;
	}

	tst_res(TFAIL, "shm_perm.mode=%04o, expected %i",
		ds->shm_perm.mode, exp_mode);
}

static void verify_shmset(void)
{
	struct shmid_ds ds;
	unsigned short old_mode;
	time_t old_ctime;

	SAFE_SHMCTL(shm_id, IPC_STAT, &ds);

	old_mode = ds.shm_perm.mode;
	old_ctime = ds.shm_ctime;

	check_mode(&ds, 0666);

	sleep(1);

	ds.shm_perm.mode &= ~0066;

	if (test_ipc_set(&ds))
		return;

	memset(&ds, 0, sizeof(ds));
	SAFE_SHMCTL(shm_id, IPC_STAT, &ds);
	check_mode(&ds, old_mode & ~0066);

	if (ds.shm_ctime <= old_ctime || ds.shm_ctime > old_ctime + 10) {
		tst_res(TFAIL, "shm_ctime not updated old %li new %li",
			(long)old_ctime, (long)ds.shm_ctime);
	} else {
		tst_res(TPASS, "shm_ctime updated correctly diff=%li",
			(long)(ds.shm_ctime - old_ctime));
	}

	ds.shm_perm.mode = old_mode;
	if (test_ipc_set(&ds))
		return;

	memset(&ds, 0, sizeof(ds));
	SAFE_SHMCTL(shm_id, IPC_STAT, &ds);
	check_mode(&ds, old_mode & MODE_MASK);
}

static void setup(void)
{
	shm_id = SAFE_SHMGET(IPC_PRIVATE, SHM_SIZE, IPC_CREAT | 0666);
}

static void cleanup(void)
{
	if (shm_id >= 0)
		SAFE_SHMCTL(shm_id, IPC_RMID, NULL);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_shmset,
};
