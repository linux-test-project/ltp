// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2009
 *				Veerendra C <vechandr@in.ibm.com>
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Test SysV IPC message passing through different namespaces.
 *
 * [Algorithm]
 *
 * In parent process create a new mesgq with a specific key.
 * In cloned process try to access the created mesgq.
 *
 * Test will PASS if the mesgq is readable when flag is None.
 * Test will FAIL if the mesgq is readable when flag is Unshare or Clone or
 * the message received is wrong.
 */

#define _GNU_SOURCE

#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/types.h>
#include "tst_safe_sysv_ipc.h"
#include "tst_test.h"
#include "common.h"

#define KEY_VAL 154326L
#define MSG_TYPE 5
#define MSG_TEXT "My message!"

static char *str_op;
static int use_clone;
static int ipc_id = -1;

struct msg_buf {
	long mtype;
	char mtext[80];
};

static int check_mesgq(LTP_ATTRIBUTE_UNUSED void *vtest)
{
	int id, n;
	struct msg_buf msg = {};

	id = msgget(KEY_VAL, 0);

	if (id < 0) {
		if (use_clone == T_NONE)
			tst_res(TFAIL, "Plain cloned process didn't find mesgq");
		else
			tst_res(TPASS, "%s: container didn't find mesgq", str_op);

		return 0;
	}

	if (use_clone == T_NONE) {
		tst_res(TPASS, "Plain cloned process found mesgq inside container");

		n = SAFE_MSGRCV(id, &msg, sizeof(msg.mtext), MSG_TYPE, 0);

		tst_res(TINFO, "Mesg read of %d bytes, Type %ld, Msg: %s", n, msg.mtype, msg.mtext);

		if (strcmp(msg.mtext, MSG_TEXT))
			tst_res(TFAIL, "Received the wrong text message");

		return 0;
	}

	tst_res(TFAIL, "%s: container init process found mesgq", str_op);
	return 0;
}

static void run(void)
{
	struct msg_buf msg = {
		.mtype = MSG_TYPE,
		.mtext = MSG_TEXT,
	};

	if (use_clone == T_NONE)
		SAFE_MSGSND(ipc_id, &msg, strlen(msg.mtext), 0);

	tst_res(TINFO, "mesgq namespaces test: %s", str_op);

	clone_unshare_test(use_clone, CLONE_NEWIPC, check_mesgq, NULL);
}

static void setup(void)
{
	use_clone = get_clone_unshare_enum(str_op);

	if (use_clone != T_NONE)
		check_newipc();

	ipc_id = SAFE_MSGGET(KEY_VAL, IPC_CREAT | IPC_EXCL | 0600);
}

static void cleanup(void)
{
	if (ipc_id != -1) {
		tst_res(TINFO, "Destroying message queue");
		SAFE_MSGCTL(ipc_id, IPC_RMID, NULL);
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
