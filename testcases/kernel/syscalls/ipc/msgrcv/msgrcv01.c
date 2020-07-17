// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * msgrcv01 - test that msgrcv() receives the expected message
 */

#include <string.h>
#include <sys/wait.h>
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "libnewipc.h"

static key_t msgkey;
static int queue_id = -1;
static struct buf {
	long type;
	char mtext[MSGSIZE];
} rcv_buf, snd_buf = {MSGTYPE, "hello"};

static void verify_msgrcv(void)
{
	SAFE_MSGSND(queue_id, &snd_buf, MSGSIZE, 0);

	TEST(msgrcv(queue_id, &rcv_buf, MSGSIZE, 1, 0));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "msgrcv failed");
		return;
	}
	if (strcmp(rcv_buf.mtext, snd_buf.mtext) == 0)
		tst_res(TPASS, "message received(%s) = message sent(%s)",
			rcv_buf.mtext, snd_buf.mtext);
	else
		tst_res(TFAIL, "message received(%s) != message sent(%s)",
			rcv_buf.mtext, snd_buf.mtext);
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
	.test_all = verify_msgrcv,
	.needs_tmpdir = 1,
};
