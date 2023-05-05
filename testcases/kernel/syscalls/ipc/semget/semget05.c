// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 */

/*\
 * [Description]
 *
 * Test for ENOSPC error.
 *
 * ENOSPC - a semaphore set exceed the maximum number of semaphore sets(SEMMNI)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include "lapi/sem.h"
#include "tst_test.h"
#include "libnewipc.h"
#include "tst_safe_sysv_ipc.h"

static int *sem_id_arr;
static int maxsems, array_cnt, used_cnt;
static key_t semkey;

static void verify_semget(void)
{
	TST_EXP_FAIL2(semget(semkey + maxsems, PSEMS, IPC_CREAT | IPC_EXCL | SEM_RA),
		ENOSPC, "semget(%i, %i, %i)", semkey + maxsems, PSEMS,
		IPC_CREAT | IPC_EXCL | SEM_RA);
}

static void setup(void)
{
	int res, num;

	semkey = GETIPCKEY();
	used_cnt = GET_USED_ARRAYS();
	tst_res(TINFO, "Current environment %d semaphore arrays are already in use",
		used_cnt);
	SAFE_FILE_SCANF("/proc/sys/kernel/sem", "%*d %*d %*d %d", &maxsems);

	/* Prevent timeout due to high semaphore array limit */
	tst_set_max_runtime(maxsems / 200);

	sem_id_arr = SAFE_MALLOC((maxsems - used_cnt) * sizeof(int));
	for (num = 0; num < maxsems - used_cnt; num++) {
		res = semget(semkey + num, PSEMS, IPC_CREAT | IPC_EXCL | SEM_RA);
		if (res == -1)
			tst_brk(TBROK | TERRNO, "semget failed unexpectedly");

		sem_id_arr[array_cnt++] = res;
	}
	tst_res(TINFO, "The maximum number of semaphore arrays (%d) has been reached",
		maxsems);
}

static void cleanup(void)
{
	int num;

	if (!sem_id_arr)
		return;

	for (num = 0; num < array_cnt; num++)
		SAFE_SEMCTL(sem_id_arr[num], PSEMS, IPC_RMID);

	free(sem_id_arr);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_semget,
	.save_restore = (const struct tst_path_val[]){
		{"/proc/sys/kernel/sem", NULL,
			TST_SR_TCONF_MISSING | TST_SR_SKIP_RO},
		{}
	}
};
