// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014-2020 Fujitsu Ltd.
 * Author: Xiaoguang Wang <wangxg.fnst@cn.fujitsu.com>
 *
 * Basic test for msgrcv(2) using MSG_EXCEPT, MSG_NOERROR
 */

#define  _GNU_SOURCE
#include <sys/wait.h>
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "libnewipc.h"

#define MSGTYPE1	1
#define MSGTYPE2	2
#define MSG1	"messagetype1"
#define MSG2	"messagetype2"

static key_t msgkey;
static int queue_id = -1;
static struct buf {
	long type;
	char mtext[MSGSIZE];
} rcv_buf, snd_buf[2] = {
	{MSGTYPE1, MSG1},
	{MSGTYPE2, MSG2}
};

static void cleanup(void)
{
	if (queue_id != -1)
		SAFE_MSGCTL(queue_id, IPC_RMID, NULL);
}

static void test_msg_except(void)
{
	queue_id = SAFE_MSGGET(msgkey, IPC_CREAT | IPC_EXCL | MSG_RW);
	SAFE_MSGSND(queue_id, &snd_buf[0], MSGSIZE, 0);
	SAFE_MSGSND(queue_id, &snd_buf[1], MSGSIZE, 0);

	memset(&rcv_buf, 0, sizeof(rcv_buf));
	TEST(msgrcv(queue_id, &rcv_buf, MSGSIZE, MSGTYPE2, MSG_EXCEPT));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "msgrcv(MSG_EXCEPT) failed");
		cleanup();
		return;
	}
	tst_res(TPASS, "msgrcv(MSG_EXCEPT) succeeded");
	if (strcmp(rcv_buf.mtext, MSG1) == 0 && rcv_buf.type == MSGTYPE1)
		tst_res(TPASS, "msgrcv(MSG_EXCEPT) excepted MSGTYPE2 and only got MSGTYPE1");
	else
		tst_res(TFAIL, "msgrcv(MSG_EXCEPT) didn't get MSGTYPE1 message");
	SAFE_MSGCTL(queue_id, IPC_RMID, NULL);
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
		cleanup();
		return;
	}
	tst_res(TPASS, "msgrcv(MSG_NOERROR) succeeded");
	if (strncmp(rcv_buf.mtext, MSG1, msg_len) == 0 && rcv_buf.type == MSGTYPE1)
		tst_res(TPASS, "msgrcv(MSG_NOERROR) truncated message text correctly");
	else
		tst_res(TFAIL, "msgrcv(MSG_NOERROR) truncated message text incorrectly");
	SAFE_MSGCTL(queue_id, IPC_RMID, NULL);
}

static void setup(void)
{
	msgkey = GETIPCKEY();
}

static void (*testfunc[])(void) = {test_msg_except, test_msg_noerror};

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
