// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Red Hat, Inc.
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Test if SysV IPC shared memory is properly working between two different
 * namespaces.
 *
 * [Algorithm]
 *
 * 1. Clones two child processes with CLONE_NEWIPC flag, each child
 *    allocates System V shared memory segment (shm) with the _identical_
 *    key and attaches that segment into its address space.
 * 2. Child1 writes into the shared memory segment.
 * 3. Child2 writes into the shared memory segment.
 * 4. Writes to the shared memory segment with the identical key but from
 *    two different IPC namespaces should not interfere with each other
 *    and so child1 checks whether its shared segment wasn't changed
 *    by child2, if it wasn't test passes, otherwise test fails.
 */

#define _GNU_SOURCE

#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/types.h>
#include "tst_safe_sysv_ipc.h"
#include "tst_test.h"
#include "common.h"

#define TESTKEY 124426L
#define SHMSIZE 50

static int chld1_shm(LTP_ATTRIBUTE_UNUSED void *arg)
{
	int id;
	char *shmem;

	id = SAFE_SHMGET(TESTKEY, SHMSIZE, IPC_CREAT);

	shmem = SAFE_SHMAT(id, NULL, 0);
	*shmem = 'A';

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	if (*shmem != 'A')
		tst_res(TFAIL, "shared memory leak between namespaces");
	else
		tst_res(TPASS, "shared memory didn't leak between namespaces");

	TST_CHECKPOINT_WAKE(0);

	SAFE_SHMDT(shmem);
	SAFE_SHMCTL(id, IPC_RMID, NULL);

	return 0;
}

static int chld2_shm(LTP_ATTRIBUTE_UNUSED void *arg)
{
	int id;
	char *shmem;

	id = SAFE_SHMGET(TESTKEY, SHMSIZE, IPC_CREAT);

	shmem = SAFE_SHMAT(id, NULL, 0);

	TST_CHECKPOINT_WAIT(0);

	*shmem = 'B';

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	SAFE_SHMDT(shmem);
	SAFE_SHMCTL(id, IPC_RMID, NULL);

	return 0;
}

static void run(void)
{
	clone_unshare_test(T_CLONE, CLONE_NEWIPC, chld1_shm, NULL);
	clone_unshare_test(T_CLONE, CLONE_NEWIPC, chld2_shm, NULL);
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
