// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2009
 *				Veerendra C <vechandr@in.ibm.com>
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Test SysV IPC semaphore usage between namespaces.
 *
 * [Algorithm]
 *
 * In parent process create a new semaphore with a specific key.
 * In cloned process, try to access the created semaphore
 *
 * Test PASS if the semaphore is readable when flag is None.
 * Test FAIL if the semaphore is readable when flag is Unshare or Clone.
 */

#define _GNU_SOURCE

#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/sem.h>
#include "tst_safe_sysv_ipc.h"
#include "tst_test.h"
#include "common.h"

#define MY_KEY 154326L

static char *str_op;
static int use_clone;
static int ipc_id = -1;

static void check_semaphore(void)
{
	int id;

	id = semget(MY_KEY, 1, 0);

	if (id < 0) {
		if (use_clone == T_NONE)
			tst_res(TFAIL, "Plain cloned process didn't find semaphore");
		else
			tst_res(TPASS, "%s: container didn't find semaphore", str_op);

		return;
	}

	tst_res(TINFO, "PID %d: fetched existing semaphore..id = %d", getpid(), id);

	if (use_clone == T_NONE)
		tst_res(TPASS, "Plain cloned process found semaphore inside container");
	else
		tst_res(TFAIL, "%s: Container init process found semaphore", str_op);
}

static void run(void)
{
	clone_unshare_test(use_clone, CLONE_NEWIPC, check_semaphore);
}

static void setup(void)
{
	use_clone = get_clone_unshare_enum(str_op);
	ipc_id = SAFE_SEMGET(MY_KEY, 1, IPC_CREAT | IPC_EXCL | 0666);
}

static void cleanup(void)
{
	if (ipc_id != -1) {
		tst_res(TINFO, "Destroying semaphore");
		SAFE_SEMCTL(ipc_id, IPC_RMID, 0);
	}
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.forks_child = 1,
	.options = (struct tst_option[]) {
		{ "m:", &str_op, "Test execution mode <clone|unshare|none>" },
		{},
	},
};
