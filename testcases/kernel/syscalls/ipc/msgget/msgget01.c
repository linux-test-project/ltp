// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 */

/*\
 * Create a message queue, write a message to it and
 * read it back.
 */

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "libnewipc.h"

static int queue_id = -1;
static key_t msgkey;

static struct buf {
	long type;
	char text[MSGSIZE];
} rcv_buf, snd_buf = {MSGTYPE, "hello, world"};

static void verify_msgget(void)
{
	TEST(msgget(msgkey, IPC_CREAT | MSG_RW));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "msgget() failed");
		return;
	}

	queue_id = TST_RET;

	SAFE_MSGSND(queue_id, &snd_buf, MSGSIZE, 0);

	SAFE_MSGRCV(queue_id, &rcv_buf, MSGSIZE, MSGTYPE, IPC_NOWAIT);

	if (strcmp(snd_buf.text, rcv_buf.text) == 0)
		tst_res(TPASS, "message received = message sent");
	else
		tst_res(TFAIL, "message received != message sent");
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
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_msgget,
	.needs_tmpdir = 1
};
