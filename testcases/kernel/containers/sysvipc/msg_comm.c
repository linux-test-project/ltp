// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Red Hat, Inc.
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Test SysV IPC message passing through different processes.
 *
 * [Algorithm]
 *
 * 1. Clones two child processes with CLONE_NEWIPC flag, each child
 *    gets System V message queue (msg) with the _identical_ key.
 * 2. Child1 appends a message with identifier #1 to the message queue.
 * 3. Child2 appends a message with identifier #2 to the message queue.
 * 4. Appends to the message queue with the identical key but from
 *    two different IPC namespaces should not interfere with each other
 *    and so child1 checks whether its message queue doesn't contain
 *    a message with identifier #2, if it doesn't test passes, otherwise
 *    test fails.
 */

#define _GNU_SOURCE

#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/types.h>
#include "tst_safe_sysv_ipc.h"
#include "tst_test.h"
#include "common.h"

#define TESTKEY 124426L

struct sysv_msg {
	long mtype;
	char mtext[1];
};

static int chld1_msg(LTP_ATTRIBUTE_UNUSED void *arg)
{
	int id;
	struct sysv_msg m = {
		.mtype = 1,
		.mtext = "A",
	};
	struct sysv_msg rec;

	id = SAFE_MSGGET(TESTKEY, IPC_CREAT | 0600);

	SAFE_MSGSND(id, &m, sizeof(m.mtext), 0);

	TST_CHECKPOINT_WAIT(0);

	TEST(msgrcv(id, &rec, sizeof(rec.mtext), 2, IPC_NOWAIT));
	if (TST_RET < 0 && TST_ERR != ENOMSG)
		tst_brk(TBROK | TERRNO, "msgrcv error");

	/* if child1 message queue has changed (by child2) report fail */
	if (TST_RET > 0)
		tst_res(TFAIL, "messages leak between namespacess");
	else
		tst_res(TPASS, "messages does not leak between namespaces");

	TST_CHECKPOINT_WAKE(0);

	SAFE_MSGCTL(id, IPC_RMID, NULL);

	return 0;
}

static int chld2_msg(LTP_ATTRIBUTE_UNUSED void *arg)
{
	int id;
	struct sysv_msg m = {
		.mtype = 2,
		.mtext = "B",
	};

	id = SAFE_MSGGET(TESTKEY, IPC_CREAT | 0600);

	SAFE_MSGSND(id, &m, sizeof(m.mtext), 0);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	SAFE_MSGCTL(id, IPC_RMID, NULL);

	return 0;
}

static void run(void)
{
	clone_unshare_test(T_CLONE, CLONE_NEWIPC, chld1_msg, NULL);
	clone_unshare_test(T_CLONE, CLONE_NEWIPC, chld2_msg, NULL);
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
