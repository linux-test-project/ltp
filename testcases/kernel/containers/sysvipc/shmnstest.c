// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2007
 *				Serge Hallyn <serue@us.ibm.com>
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test if SysV IPC shared memory with a specific key is shared between
 * processes and namespaces.
 */

#define _GNU_SOURCE

#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/types.h>
#include "tst_safe_sysv_ipc.h"
#include "tst_test.h"
#include "common.h"

#define TESTKEY 0xEAEAEA

static char *str_op;
static int use_clone;
static int ipc_id = -1;

static void check_shmid(void)
{
	TEST(shmget(TESTKEY, 100, 0));
	if (TST_RET < 0) {
		if (use_clone == T_NONE)
			tst_res(TFAIL, "plain cloned process didn't find shmid");
		else
			tst_res(TPASS, "%s: child process didn't find shmid", str_op);
	} else {
		if (use_clone == T_NONE)
			tst_res(TPASS, "plain cloned process found shmid");
		else
			tst_res(TFAIL, "%s: child process found shmid", str_op);
	}
}

static void run(void)
{
	clone_unshare_test(use_clone, CLONE_NEWIPC, check_shmid);
}

static void setup(void)
{
	use_clone = get_clone_unshare_enum(str_op);
	ipc_id = shmget(TESTKEY, 100, IPC_CREAT);
}

static void cleanup(void)
{
	if (ipc_id != -1) {
		tst_res(TINFO, "Destroying shared memory");
		SAFE_SHMCTL(ipc_id, IPC_RMID, NULL);
	}
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.needs_root = 1,
	.needs_checkpoints = 1,
	.options = (struct tst_option[]) {
		{ "m:", &str_op, "Test execution mode <clone|unshare|none>" },
		{},
	},
};
