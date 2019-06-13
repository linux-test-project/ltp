// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 */

/*
 * DESCRIPTION
 * test that msgsnd() enqueues a message correctly.
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "libnewipc.h"

static key_t msgkey;
static int queue_id = -1;
static struct buf {
	long type;
	char text[MSGSIZE];
} rcv_buf, snd_buf = {MSGTYPE, "hello"};

static void verify_msgsnd(void)
{
	struct msqid_ds qs_buf;

	TEST(msgsnd(queue_id, &snd_buf, MSGSIZE, 0));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "msgsnd() failed");
		return;
	}

	SAFE_MSGCTL(queue_id, IPC_STAT, &qs_buf);

	if (qs_buf.msg_cbytes == MSGSIZE && qs_buf.msg_qnum == 1)
		tst_res(TPASS, "queue bytes and number of queues matched");
	else
		tst_res(TFAIL, "queue bytes or number of queues mismatched");

	SAFE_MSGRCV(queue_id, &rcv_buf, MSGSIZE, 1, 0);
}

static void setup(void)
{
	msgkey = GETIPCKEY();

	queue_id = SAFE_MSGGET(msgkey, IPC_CREAT | IPC_EXCL | MSG_RW);
}

static void cleanup(void)
{
	if (queue_id != -1)
		SAFE_MSGCTL(queue_id, IPC_RMID, NULL);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_msgsnd,
	.needs_tmpdir = 1
};
