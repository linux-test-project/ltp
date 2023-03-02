// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2009
 *				Veerendra C <vechandr@in.ibm.com>
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Test if SysV IPC shared memory is properly used between two namespaces.
 *
 * [Algorithm]
 *
 * Create two 'containers'.
 * In container1, create Shared Memory segment with a specific key.
 * In container2, try to access the MQ created in container1.
 *
 * Test is PASS if flag = none and the shmem seg is accessible in container2.
 * If flag = unshare/clone and the shmem seg is not accessible in container2.
 * If shmem seg is not accessible in container2, creates new shmem with same
 * key to double check isloation in IPCNS.
 *
 * Test is FAIL if flag = none and the shmem seg is not accessible, if
 * flag = unshare/clone and shmem seg is accessible in container2 or if the new
 * shmem seg creation Fails.
 */

#define _GNU_SOURCE

#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/types.h>
#include "tst_safe_sysv_ipc.h"
#include "tst_test.h"
#include "common.h"

#define TESTKEY 124426L

static char *str_op;
static int use_clone;

static void check_shmem1(void)
{
	int id;

	id = SAFE_SHMGET(TESTKEY, 100, IPC_CREAT);

	tst_res(TINFO, "container1: able to create shared mem segment");

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	SAFE_SHMCTL(id, IPC_RMID, NULL);
}

static void check_shmem2(void)
{
	TST_CHECKPOINT_WAIT(0);

	TEST(shmget(TESTKEY, 100, 0));

	if (TST_RET < 0) {
		if (use_clone == T_NONE)
			tst_res(TFAIL, "Plain cloned process didn't find shmem segment");
		else
			tst_res(TPASS, "%s: in namespace2 unable to access the shmem seg created in namespace1", str_op);
	} else {
		if (use_clone == T_NONE)
			tst_res(TPASS, "Plain cloned process able to access shmem segment created");
		else
			tst_res(TFAIL, "%s: in namespace2 found the shmem segment created in namespace1", str_op);
	}

	TST_CHECKPOINT_WAKE(0);
}

static void run(void)
{
	clone_unshare_test(use_clone, CLONE_NEWIPC, check_shmem1);
	clone_unshare_test(use_clone, CLONE_NEWIPC, check_shmem2);
}

static void setup(void)
{
	use_clone = get_clone_unshare_enum(str_op);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.forks_child = 1,
	.needs_root = 1,
	.needs_checkpoints = 1,
	.options = (struct tst_option[]) {
		{ "m:", &str_op, "Test execution mode <clone|unshare|none>" },
		{},
	},
};
