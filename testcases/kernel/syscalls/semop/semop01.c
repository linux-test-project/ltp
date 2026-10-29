// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Test that semop() basic functionality is correct
 *
 * Copyright (c) International Business Machines Corp., 2001
 *	03/2001 - Written by Wayne Boyer
 *	17/01/02 - Modified. Manoj Iyer, IBM Austin. TX. manjo@austin.ibm.com
 */

#include <stdlib.h>
#include "tst_test.h"
#include "tse_newipc.h"
#include "lapi/sem.h"
#include "semop.h"

#define NSEMS 4

static int sem_id = -1;
static key_t semkey;

static unsigned short int sarr[PSEMS];
static union semun get_arr = {.array = sarr};
static struct sembuf sops[PSEMS];
static struct tst_ts timeout;

static struct test_case_t {
	struct tst_ts *to;
} tc[] = {
	{NULL},
	{&timeout}
};

static void run(unsigned int n)
{
	struct time64_variants *tv = &variants[tst_variant];
	union semun arr = { .val = 0 };
	int fail = 0;
	int i;

	TEST(call_semop(tv, sem_id, sops, NSEMS, tst_ts_get(tc[n].to)));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "semop() failed");
		return;
	}

	if (semctl(sem_id, 0, GETALL, get_arr) == -1)
		tst_brk(TBROK | TERRNO, "semctl(%i, 0, GETALL, ...)", sem_id);

	for (i = 0; i < NSEMS; i++) {
		if (get_arr.array[i] != i * i) {
			fail = 1;
		}
	}

	if (fail)
		tst_res(TFAIL, "semaphore values are wrong");
	else
		tst_res(TPASS, "semaphore values are correct");

	for (i = 0; i < NSEMS; i++) {
		if (semctl(sem_id, i, SETVAL, arr) == -1)
			tst_brk(TBROK | TERRNO, "semctl(%i, %i, SETVAL, ...)", sem_id, i);
	}
}

static void setup(void)
{
	struct time64_variants *tv = &variants[tst_variant];
	int i;

	tst_res(TINFO, "Testing variant: %s", tv->desc);
	semop_supported_by_kernel(tv);

	timeout.type = tv->ts_type;
	tst_ts_set_sec(&timeout, 0);
	tst_ts_set_nsec(&timeout, 10000);

	semkey = GETIPCKEY();

	sem_id = semget(semkey, PSEMS, IPC_CREAT | IPC_EXCL | SEM_RA);
	if (sem_id == -1)
		tst_brk(TBROK | TERRNO, "couldn't create semaphore in setup");

	for (i = 0; i < NSEMS; i++) {
		sops[i].sem_num = i;
		sops[i].sem_op = i * i;
		sops[i].sem_flg = SEM_UNDO;
	}
}

static void cleanup(void)
{
	if (sem_id != -1) {
		if (semctl(sem_id, 0, IPC_RMID) == -1)
			tst_res(TWARN, "semaphore deletion failed.");
	}
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tc),
	.test_variants = ARRAY_SIZE(variants),
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
