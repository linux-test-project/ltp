// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*\
 * It is a basic test for shm_next_id.
 *
 * shm_next_id specifies desired id for next allocated IPC shared memory. By
 * default it's equal to -1, which means generic allocation logic.
 * Possible values to set are in range {0..INT_MAX}.
 * The value will be set back to -1 by kernel after successful IPC object
 * allocation.
 */

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "libnewipc.h"

#define NEXT_ID_PATH "/proc/sys/kernel/shm_next_id"
static int shm_id, pid;
static key_t shmkey;

static void verify_shmget(void)
{
	SAFE_FILE_PRINTF(NEXT_ID_PATH, "%d", pid);

	shm_id = SAFE_SHMGET(shmkey, SHM_SIZE, SHM_RW | IPC_CREAT);
	if (shm_id == pid)
		tst_res(TPASS, "shm_next_id succeeded, shm id %d", pid);
	else
		tst_res(TFAIL, "shm_next_id failed, expected id %d, but got %d", pid, shm_id);

	TST_ASSERT_INT(NEXT_ID_PATH, -1);
	SAFE_SHMCTL(shm_id, IPC_RMID, NULL);
	pid++;
}

static void setup(void)
{
	shmkey = GETIPCKEY();
	pid = getpid();
}

static void cleanup(void)
{
	if (shm_id != -1)
		SAFE_SHMCTL(shm_id, IPC_RMID, NULL);
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
