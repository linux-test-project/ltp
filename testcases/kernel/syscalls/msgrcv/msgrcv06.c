// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * msgrcv error test for EIDRM.
 */

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "tse_newipc.h"

static key_t msgkey;
static int queue_id = -1;
static struct buf {
	long type;
	char text[MSGSIZE];
} rcv_buf = {1, "hello"};

static void verify_msgrcv(void)
{
	TST_EXP_FAIL2(msgrcv(queue_id, &rcv_buf, MSGSIZE, 1, 0), EIDRM,
		"msgrcv(%i, %p, %d, 1, 0)", queue_id, &rcv_buf, MSGSIZE);
}

static void do_test(void)
{
	int pid;

	queue_id = SAFE_MSGGET(msgkey, IPC_CREAT | IPC_EXCL | MSG_RW);
	pid = SAFE_FORK();
	if (pid == 0) {
		verify_msgrcv();
		exit(0);
	}
	TST_PROCESS_STATE_WAIT(pid, 'S', 0);
	SAFE_MSGCTL(queue_id, IPC_RMID, NULL);
	tst_reap_children();
}

static void setup(void)
{
	msgkey = GETIPCKEY();
}

static void cleanup(void)
{
	if (queue_id != -1)
		SAFE_MSGCTL(queue_id, IPC_RMID, NULL);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.forks_child = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = do_test,
};
