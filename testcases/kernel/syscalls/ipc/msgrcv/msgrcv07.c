// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014-2020 Fujitsu Ltd.
 * Author: Xiaoguang Wang <wangxg.fnst@cn.fujitsu.com>
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 *
 * Basic test for msgrcv(2) using MSG_EXCEPT, MSG_NOERROR, MSG_COPY and
 * different msg_typ(zero,positive,negative).
 *
 * * With MSG_EXCEPT flag any message type but the one passed to the function
 *   is received.
 *
 * * With MSG_NOERROR and buffer size less than message size only part of the
 *   buffer is received.
 *
 * * With MSG_COPY and IPC_NOWAIT flag read the msg but don't destroy it in
 *   msg queue.
 *
 * * With msgtyp is 0, then the first message in the queue is read.
 *
 * * With msgtyp is greater than 0, then the first message in the queue of type
 *   msgtyp is read.
 *
 * * With msgtyp is less than 0, then the first message in the queue with the
 *   lowest type less than or equal to absolute value of msgtyp is received.
 */

#define  _GNU_SOURCE
#include <sys/wait.h>
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "libnewipc.h"
#include "lapi/msg.h"

#define MSGTYPE1	1
#define MSGTYPE2	2
#define MSG1	"messagetype1"
#define MSG2	"messagetype2"

static key_t msgkey;
static int queue_id = -1, msg_copy_sup;
static struct buf {
	long type;
	char mtext[MSGSIZE];
} rcv_buf, snd_buf[2] = {
	{MSGTYPE1, MSG1},
	{MSGTYPE2, MSG2}
};

static void prepare_queue(void)
{
	queue_id = SAFE_MSGGET(msgkey, IPC_CREAT | IPC_EXCL | MSG_RW);
	SAFE_MSGSND(queue_id, &snd_buf[0], MSGSIZE, 0);
	SAFE_MSGSND(queue_id, &snd_buf[1], MSGSIZE, 0);
	memset(&rcv_buf, 0, sizeof(rcv_buf));
}

static void test_msg_except(void)
{
	prepare_queue();

	TEST(msgrcv(queue_id, &rcv_buf, MSGSIZE, MSGTYPE2, MSG_EXCEPT));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "msgrcv(MSG_EXCEPT) failed");
		goto exit;
	}

	tst_res(TPASS, "msgrcv(MSG_EXCEPT) succeeded");

	if (strcmp(rcv_buf.mtext, MSG1) == 0 && rcv_buf.type == MSGTYPE1)
		tst_res(TPASS, "MSG_EXCEPT excepted MSGTYPE2 and got MSGTYPE1");
	else
		tst_res(TFAIL, "MSG_EXCEPT didn't get MSGTYPE1 message");

exit:
	SAFE_MSGCTL(queue_id, IPC_RMID, NULL);
	queue_id = -1;
}

static void test_msg_noerror(void)
{
	int msg_len;

	queue_id = SAFE_MSGGET(msgkey, IPC_CREAT | IPC_EXCL | MSG_RW);
	SAFE_MSGSND(queue_id, &snd_buf[0], MSGSIZE, 0);
	msg_len = sizeof(MSG1) / 2;
	memset(&rcv_buf, 0, sizeof(rcv_buf));

	TEST(msgrcv(queue_id, &rcv_buf, msg_len, MSGTYPE1, MSG_NOERROR));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "msgrcv(MSG_NOERROR) failed");
		goto exit;
	}

	tst_res(TPASS, "msgrcv(MSG_NOERROR) succeeded");

	if (strncmp(rcv_buf.mtext, MSG1, msg_len) == 0 && rcv_buf.type == MSGTYPE1)
		tst_res(TPASS, "MSG_NOERROR truncated message correctly");
	else
		tst_res(TFAIL, "MSG_NOERROR truncated message incorrectly");

exit:
	SAFE_MSGCTL(queue_id, IPC_RMID, NULL);
	queue_id = -1;
}

static void test_msg_copy(void)
{
	struct msqid_ds buf = {0};

	if (!msg_copy_sup) {
		tst_res(TCONF, "MSG_COPY not supported");
		return;
	}

	prepare_queue();

	/*
	 * If MSG_COPY flag was specified, then mtype is interpreted as number
	 * of the message to copy.
	 */
	TEST(msgrcv(queue_id, &rcv_buf, MSGSIZE, 0, MSG_COPY | IPC_NOWAIT));
	if (TST_RET == -1) {
		if (TST_ERR == ENOSYS) {
			tst_res(TCONF,
				"MSG_COPY needs CONFIG_CHECKPORINT_RESTORE");
			msg_copy_sup = 0;
		} else {
			tst_res(TFAIL | TTERRNO, "msgrcv(0, MSG_COPY) failed");
		}
		goto exit;
	}

	tst_res(TPASS, "msgrcv(0, MSG_COPY) succeeded");

	if (strcmp(rcv_buf.mtext, MSG1) == 0 && rcv_buf.type == MSGTYPE1)
		tst_res(TPASS, "MSG_COPY got MSGTYPE1 data correctly");
	else
		tst_res(TFAIL, "MSG_COPY got MSGTYPE1 data incorrectly");

	memset(&rcv_buf, 0, sizeof(rcv_buf));
	TEST(msgrcv(queue_id, &rcv_buf, MSGSIZE, 1, MSG_COPY | IPC_NOWAIT));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "msgrcv(1, MSG_COPY) failed");
		goto exit;
	}

	tst_res(TPASS, "msgrcv(1, MSG_COPY) succeeded");

	if (strcmp(rcv_buf.mtext, MSG2) == 0 && rcv_buf.type == MSGTYPE2)
		tst_res(TPASS, "MSG_COPY got MSGTYPE2 data correctly");
	else
		tst_res(TFAIL, "MSG_COPY got MSGTYPE2 data incorrectly");

	SAFE_MSGCTL(queue_id, IPC_STAT, &buf);
	if (buf.msg_qnum == 2) {
		tst_res(TPASS, "Two messages still in queue");
	} else {
		tst_res(TFAIL, "Expected 2 msgs in queue got %d",
		       (int)buf.msg_qnum);
	}

exit:
	SAFE_MSGCTL(queue_id, IPC_RMID, NULL);
	queue_id = -1;
}

static void test_zero_msgtyp(void)
{
	prepare_queue();

	TEST(msgrcv(queue_id, &rcv_buf, MSGSIZE, 0, 0));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "msgrcv(zero_msgtyp) failed");
		goto exit;
	}

	tst_res(TPASS, "msgrcv(zero_msgtyp) succeeded");

	if (strcmp(rcv_buf.mtext, MSG1) == 0 && rcv_buf.type == MSGTYPE1)
		tst_res(TPASS, "zero_msgtyp got the first message");
	else
		tst_res(TFAIL, "zero_msgtyp didn't get the first message");

exit:
	SAFE_MSGCTL(queue_id, IPC_RMID, NULL);
	queue_id = -1;
}

static void test_positive_msgtyp(void)
{
	prepare_queue();

	TEST(msgrcv(queue_id, &rcv_buf, MSGSIZE, MSGTYPE2, 0));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "msgrcv(positive_msgtyp) failed");
		goto exit;
	}

	tst_res(TPASS, "msgrcv(positive_msgtyp) succeeded");

	if (strcmp(rcv_buf.mtext, MSG2) == 0 && rcv_buf.type == MSGTYPE2) {
		tst_res(TPASS,
			"msgtyp got the first message in the queue of type msgtyp");
	} else {
		tst_res(TFAIL,
			"msgtyp didn't get the first message in the queue of type msgtyp");
	}

exit:
	SAFE_MSGCTL(queue_id, IPC_RMID, NULL);
	queue_id = -1;
}

static void test_negative_msgtyp(void)
{
	prepare_queue();

	TEST(msgrcv(queue_id, &rcv_buf, MSGSIZE, -MSGTYPE2, 0));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "msgrcv(negative_msgtyp) failed");
		goto exit;
	}

	tst_res(TPASS, "msgrcv(negative_msgtyp) succeeded");

	if (strcmp(rcv_buf.mtext, MSG1) == 0 && rcv_buf.type == MSGTYPE1) {
		tst_res(TPASS,
			"-msgtyp got the first message in the queue with the lowest type");
	} else {
		tst_res(TFAIL,
			"-msgtyp didn't get the first message in the queue with the lowest type");
	}

exit:
	SAFE_MSGCTL(queue_id, IPC_RMID, NULL);
	queue_id = -1;
}

static void cleanup(void)
{
	if (queue_id != -1)
		SAFE_MSGCTL(queue_id, IPC_RMID, NULL);
}

static void setup(void)
{
	msgkey = GETIPCKEY();

	msg_copy_sup = 1;
}

static void (*testfunc[])(void) = {test_msg_except, test_msg_noerror,
				   test_msg_copy, test_zero_msgtyp,
				   test_positive_msgtyp, test_negative_msgtyp};

static void verify_msgcrv(unsigned int n)
{
	(*testfunc[n])();
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_msgcrv,
	.tcnt = ARRAY_SIZE(testfunc),
};
