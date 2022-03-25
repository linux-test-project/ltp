// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2009
 *				Veerendra C <vechandr@in.ibm.com>
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Test SysV IPC semaphore usage between namespaces and processes.
 *
 * [Algorithm]
 *
 * Create 2 'containers'
 * In container1 create semaphore with a specific key and lock it
 * In container2 try to access the semaphore created in container1 and try to
 * unlock it.
 *
 * If mode = None, test will PASS when semaphore created process1 can be read
 * and unlocked from process2.
 * If mode = Clone, test will PASS when semaphore created in container1 can't
 * be accessed from container2.
 * If mode = Unshare, test will PASS when semaphore created in container2 can't
 * be accessed from container2.
 */

#define _GNU_SOURCE

#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/sem.h>
#include "tst_safe_sysv_ipc.h"
#include "tst_test.h"
#include "common.h"

#define MY_KEY 124326L

static char *str_op;
static int use_clone;

static int check_sem1(LTP_ATTRIBUTE_UNUSED void *vtest)
{
	int id;
	struct sembuf sm = {
		.sem_num = 0,
		.sem_op = 1,
		.sem_flg = SEM_UNDO,
	};

	id = SAFE_SEMGET(MY_KEY, 1, IPC_CREAT | IPC_EXCL | 0666);

	tst_res(TINFO, "%s: created key in child1", str_op);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	tst_res(TINFO, "Lock semaphore in container1");

	SAFE_SEMOP(id, &sm, 1);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	SAFE_SEMCTL(id, IPC_RMID, 0);

	return 0;
}

static int check_sem2(LTP_ATTRIBUTE_UNUSED void *vtest)
{
	int id;
	struct sembuf sm = {
		.sem_num = 0,
		.sem_op = -1,
		.sem_flg = IPC_NOWAIT | SEM_UNDO,
	};

	TST_CHECKPOINT_WAIT(0);

	tst_res(TINFO, "%s: reading key in child2", str_op);

	id = semget(MY_KEY, 1, 0);
	if (id >= 0) {
		if (use_clone == T_NONE)
			tst_res(TPASS, "Plain cloned process able to access the semaphore created");
		else
			tst_res(TFAIL, "%s: In namespace2 found semaphore created in namespace1", str_op);
	} else {
		if (use_clone == T_NONE)
			tst_res(TFAIL, "Plain cloned process didn't find semaphore");
		else
			tst_res(TPASS, "%s: In namespace2 unable to access semaphore created in namespace1", str_op);
	}

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	if (id >= 0) {
		tst_res(TINFO, "Trying to unlock semaphore in container2");
		TEST(semop(id, &sm, 1));

		if (TST_RET >= 0) {
			if (use_clone == T_NONE)
				tst_res(TPASS, "Plain cloned process able to unlock semaphore");
			else
				tst_res(TFAIL, "%s: In namespace2 able to unlock the semaphore created in an namespace1", str_op);
		} else {
			if (use_clone == T_NONE)
				tst_res(TFAIL, "Plain cloned process unable to unlock semaphore");
			else
				tst_res(TPASS, "%s: In namespace2 unable to unlock the semaphore created in an namespace1", str_op);
		}
	}

	TST_CHECKPOINT_WAKE(0);

	return 0;
}

static void run(void)
{
	clone_unshare_test(use_clone, CLONE_NEWIPC, check_sem1, NULL);
	clone_unshare_test(use_clone, CLONE_NEWIPC, check_sem2, NULL);
}

static void setup(void)
{
	use_clone = get_clone_unshare_enum(str_op);

	if (use_clone != T_NONE)
		check_newipc();
}

static void cleanup(void)
{
	int id;

	id = semget(MY_KEY, 1, 0);
	if (id >= 0) {
		tst_res(TINFO, "Destroy semaphore");
		SAFE_SEMCTL(id, IPC_RMID, 0);
	}
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.options = (struct tst_option[]) {
		{ "m:", &str_op, "Test execution mode <clone|unshare|none>" },
		{},
	},
};
