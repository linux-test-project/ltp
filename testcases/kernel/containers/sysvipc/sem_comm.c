// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Red Hat, Inc.
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Test SysV IPC semaphore usage between cloned processes.
 *
 * [Algorithm]
 *
 * 1. Clones two child processes with CLONE_NEWIPC flag, each child
 *    creates System V semaphore (sem) with the _identical_ key.
 * 2. Child1 locks the semaphore.
 * 3. Child2 locks the semaphore.
 * 4. Locking the semaphore with the identical key but from two different
 *    IPC namespaces should not interfere with each other, so if child2
 *    is able to lock the semaphore (after child1 locked it), test passes,
 *    otherwise test fails.
 */

#define _GNU_SOURCE

#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/types.h>
#include "tst_safe_sysv_ipc.h"
#include "tst_test.h"
#include "lapi/sem.h"
#include "common.h"

#define TESTKEY 124426L

static int chld1_sem(LTP_ATTRIBUTE_UNUSED void *arg)
{
	int id;
	struct sembuf sm = {
		.sem_num = 0,
		.sem_op = -1,
		.sem_flg = IPC_NOWAIT,
	};
	union semun su = {
		.val = 1,
	};

	id = SAFE_SEMGET(TESTKEY, 1, IPC_CREAT);

	SAFE_SEMCTL(id, 0, SETVAL, su);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	SAFE_SEMOP(id, &sm, 1);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	SAFE_SEMCTL(id, 0, IPC_RMID);

	return 0;
}

static int chld2_sem(LTP_ATTRIBUTE_UNUSED void *arg)
{
	int id;
	struct sembuf sm = {
		.sem_num = 0,
		.sem_op = -1,
		.sem_flg = IPC_NOWAIT,
	};
	union semun su = {
		.val = 1,
	};

	TST_CHECKPOINT_WAIT(0);

	id = SAFE_SEMGET(TESTKEY, 1, IPC_CREAT);

	SAFE_SEMCTL(id, 0, SETVAL, su);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	TEST(semop(id, &sm, 1));
	if (TST_RET < 0) {
		if (TST_ERR != EAGAIN)
			tst_brk(TBROK | TERRNO, "semop error");

		tst_res(TFAIL, "semaphore decremented from different namespace");
	} else {
		tst_res(TPASS, "semaphore has not been decremented");
	}

	TST_CHECKPOINT_WAKE(0);

	SAFE_SEMCTL(id, 0, IPC_RMID);

	return 0;
}

static void run(void)
{
	clone_unshare_test(T_CLONE, CLONE_NEWIPC, chld1_sem, NULL);
	clone_unshare_test(T_CLONE, CLONE_NEWIPC, chld2_sem, NULL);
}

static void setup(void)
{
	check_newipc();
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.needs_root = 1,
	.needs_checkpoints = 1,
};
