// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*\
 * It is a basic test for shm_next_id.
 *
 * When the shared memory segment identifier that shm_next_id stored is already
 * in use, call shmget with different key just use another unused value in range
 * [0,INT_MAX]. Kernel doesn't guarantee the desired id.
 */

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "tse_newipc.h"

#define NEXT_ID_PATH "/proc/sys/kernel/shm_next_id"

static int shm_id[2], pid;
static key_t shmkey[2];

static void verify_shmget(void)
{
	SAFE_FILE_PRINTF(NEXT_ID_PATH, "%d", shm_id[0]);

	shm_id[1] = SAFE_SHMGET(shmkey[1], SHM_SIZE, IPC_CREAT | SHM_RW);
	if (shm_id[1] == shm_id[0])
		tst_res(TFAIL, "shm id %d has existed, shmget() returns the"
			" same shm id unexpectedly", shm_id[0]);
	else
		tst_res(TPASS, "shm id %d has existed, shmget() returns the"
			" new shm id %d", shm_id[0], shm_id[1]);

	SAFE_SHMCTL(shm_id[1], IPC_RMID, NULL);
}

static void setup(void)
{
	shmkey[0] = GETIPCKEY();
	shmkey[1] = GETIPCKEY();
	pid = getpid();
	SAFE_FILE_PRINTF(NEXT_ID_PATH, "%d", pid);
	shm_id[0] = SAFE_SHMGET(shmkey[0], SHM_SIZE, IPC_CREAT | SHM_RW);
}

static void cleanup(void)
{
	int i;

	for (i = 0; i < 2; i++) {
		if (shm_id[i] != -1)
			SAFE_SHMCTL(shm_id[i], IPC_RMID, NULL);
	}
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_shmget,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_CHECKPOINT_RESTORE=y",
		NULL
	},
	.needs_root = 1,
};
