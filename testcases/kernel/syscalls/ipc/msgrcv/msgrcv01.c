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
#include "tst_clocks.h"
#include "libnewipc.h"

static key_t msgkey;
static int queue_id = -1, pid;
static struct buf {
	long type;
	char mtext[MSGSIZE];
} rcv_buf, snd_buf = {MSGTYPE, "hello"};

static void verify_msgrcv(void)
{
	struct msqid_ds qs_buf;
	time_t before_rcv, after_rcv;

	SAFE_MSGSND(queue_id, &snd_buf, MSGSIZE, 0);

	before_rcv = tst_get_fs_timestamp();
	TEST(msgrcv(queue_id, &rcv_buf, MSGSIZE, 1, 0));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "msgrcv failed");
		return;
	}
	after_rcv = tst_get_fs_timestamp();

	if (strcmp(rcv_buf.mtext, snd_buf.mtext) == 0)
		tst_res(TPASS, "message received(%s) = message sent(%s)",
			rcv_buf.mtext, snd_buf.mtext);
	else
		tst_res(TFAIL, "message received(%s) != message sent(%s)",
			rcv_buf.mtext, snd_buf.mtext);

	SAFE_MSGCTL(queue_id, IPC_STAT, &qs_buf);
	if (qs_buf.msg_cbytes == 0 && qs_buf.msg_qnum == 0)
		tst_res(TPASS, "queue bytes and number of queues matched");
	else
		tst_res(TFAIL, "queue bytes or number of queues mismatched");
	if (qs_buf.msg_lrpid == pid)
		tst_res(TPASS, "PID of last msgrcv(2) matched");
	else
		tst_res(TFAIL, "PID of last msgrcv(2) mismatched");

	if (qs_buf.msg_rtime >= before_rcv && qs_buf.msg_rtime <= after_rcv) {
		tst_res(TPASS, "msg_rtime = %lu in [%lu, %lu]",
			(unsigned long)qs_buf.msg_rtime,
			(unsigned long)before_rcv, (unsigned long)after_rcv);
	} else {
		tst_res(TFAIL, "msg_rtime = %lu out of [%lu, %lu]",
			(unsigned long)qs_buf.msg_rtime,
			(unsigned long)before_rcv, (unsigned long)after_rcv);
	}
}

static void setup(void)
{
	msgkey = GETIPCKEY();
	queue_id = SAFE_MSGGET(msgkey, IPC_CREAT | IPC_EXCL | MSG_RW);
	pid = getpid();
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
