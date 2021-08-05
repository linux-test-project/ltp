// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  03/2001 - Written by Wayne Boyer
 */

/*\
 * [Description]
 *
 * Test for ENOSPC error.
 *
 * ENOSPC -  All possible shared memory segments have been taken (SHMMNI).
 */
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/shm.h>
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "libnewipc.h"

static int *queues;
static int maxshms, queue_cnt, used_cnt;
static key_t shmkey;

static void verify_shmget(void)
{
	TST_EXP_FAIL2(shmget(shmkey + maxshms, SHM_SIZE, IPC_CREAT | IPC_EXCL | SHM_RW), ENOSPC,
		"shmget(%i, %i, %i)", shmkey + maxshms, SHM_SIZE, IPC_CREAT | IPC_EXCL | SHM_RW);
}

static void setup(void)
{
	int res, num;

	shmkey = GETIPCKEY();
	used_cnt = GET_USED_SEGMENTS();
	tst_res(TINFO, "Current environment %d shared memory segments are already in use",
		used_cnt);
	SAFE_FILE_SCANF("/proc/sys/kernel/shmmni", "%i", &maxshms);

	queues = SAFE_MALLOC((maxshms - used_cnt) * sizeof(int));
	for (num = 0; num < maxshms - used_cnt; num++) {
		res = shmget(shmkey + num, SHM_SIZE, IPC_CREAT | IPC_EXCL | SHM_RW);
		if (res == -1)
			tst_brk(TBROK | TERRNO, "shmget failed unexpectedly");

		queues[queue_cnt++] = res;
	}
	tst_res(TINFO, "The maximum number of memory segments (%d) has been reached",
		maxshms);
}

static void cleanup(void)
{
	int num;

	if (!queues)
		return;

	for (num = 0; num < queue_cnt; num++)
		SAFE_SHMCTL(queues[num], IPC_RMID, NULL);

	free(queues);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_shmget,
};
