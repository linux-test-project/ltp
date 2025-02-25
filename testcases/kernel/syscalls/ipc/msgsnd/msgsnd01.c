// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Copyright (c) Linux Test Project, 2002-2024
 */

/*\
 * Verify that msgsnd(2) enqueues a message correctly.
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "tst_clocks.h"
#include "libnewipc.h"

static key_t msgkey;
static int queue_id = -1, pid;
static struct buf {
	long type;
	char text[MSGSIZE];
} rcv_buf, snd_buf = {MSGTYPE, "hello"};

static void verify_msgsnd(void)
{
	struct msqid_ds qs_buf;
	time_t before_snd, after_snd;

	before_snd = tst_fs_timestamp_start();
	TEST(msgsnd(queue_id, &snd_buf, MSGSIZE, 0));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "msgsnd() failed");
		return;
	}
	after_snd = tst_fs_timestamp_end();

	SAFE_MSGCTL(queue_id, IPC_STAT, &qs_buf);

	if (qs_buf.msg_cbytes == MSGSIZE && qs_buf.msg_qnum == 1)
		tst_res(TPASS, "queue bytes and number of queues matched");
	else
		tst_res(TFAIL, "queue bytes or number of queues mismatched");

	if (qs_buf.msg_lspid == pid)
		tst_res(TPASS, "PID of last msgsnd(2) matched");
	else
		tst_res(TFAIL, "PID of last msgsnd(2) mismatched");

	if (qs_buf.msg_stime >= before_snd && qs_buf.msg_stime <= after_snd) {
		tst_res(TPASS, "msg_stime = %lu in [%lu, %lu]",
			(unsigned long)qs_buf.msg_stime,
			(unsigned long)before_snd, (unsigned long)after_snd);
	} else {
		tst_res(TFAIL, "msg_stime = %lu out of [%lu, %lu]",
			(unsigned long)qs_buf.msg_stime,
			(unsigned long)before_snd, (unsigned long)after_snd);
	}

	SAFE_MSGRCV(queue_id, &rcv_buf, MSGSIZE, 1, 0);
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
	.test_all = verify_msgsnd,
	.needs_tmpdir = 1
};
