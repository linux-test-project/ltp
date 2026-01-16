// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Author: Feiyu Zhu <zhufy.jy@cn.fujitsu.com>
 */
/*\
 * Call msgctl() with MSG_INFO flag and check that:
 *
 * * The returned index points to a valid MSG by calling MSG_STAT_ANY
 * * Also count that valid indexes < returned max index sums up to used_ids
 * * And the data are consistent with /proc/sysvipc/msg
 *
 * There is a possible race between the call to the msgctl() and read from the
 * proc file so this test cannot be run in parallel with any IPC testcases that
 * adds or removes MSG queues.
 *
 * Note what we create a MSG segment in the test setup and send msg to make sure
 * that there is at least one during the testrun.
 *
 * Also note that for MSG_INFO the members of the msginfo structure have
 * completely different meaning than their names seems to suggest.
 */

#include <stdio.h>
#include <pwd.h>
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "tse_newipc.h"
#include "lapi/msg.h"

static int msg_id = -1;
static struct passwd *ltpuser;
static uid_t nobody_uid, root_uid;

static struct tcases {
	uid_t *uid;
	char *desc;
} tests[] = {
	{&nobody_uid, "with nobody user"},
	{&root_uid, "with root user"}
};

static void parse_proc_sysvipc(struct msginfo *info)
{
	FILE *f = fopen("/proc/sysvipc/msg", "r");
	int queue_cnt = 0;
	int msg_cnt = 0;
	int msg_bytes = 0;

	/* Eat header */
	for (;;) {
		int c = fgetc(f);

		if (c == '\n' || c == EOF)
			break;
	}

	int cbytes, msgs;

	/*
	 * Sum queue and byte for all elements listed, which should equal
	 * the data returned in the msginfo structure.
	 */
	while (fscanf(f, "%*i %*i %*i %i %i %*i %*i %*i %*i %*i %*i %*i %*i %*i",
	              &cbytes, &msgs) > 0){
		queue_cnt++;
		msg_cnt += msgs;
		msg_bytes += cbytes;
	}

	if (info->msgpool != queue_cnt) {
		tst_res(TFAIL, "msgpool = %i, expected %i",
			info->msgpool, queue_cnt);
	} else {
		tst_res(TPASS, "queue_cnt = %i", queue_cnt);
	}

	if (info->msgmap != msg_cnt) {
		tst_res(TFAIL, "msgmap = %i, expected %i",
			info->msgpool, msg_cnt);
	} else {
		tst_res(TPASS, "msg_cnt = %i", msg_cnt);
	}

	if (info->msgtql != msg_bytes) {
		tst_res(TFAIL, "msgtql = %i, expected %i",
			info->msgtql, msg_bytes);
	} else {
		tst_res(TPASS, "msg_bytes = %i", msg_bytes);
	}

	fclose(f);
}

static void verify_msgctl(unsigned int n)
{
	struct tcases *tc = &tests[n];
	int i, msgid, cnt = 0;
	struct msqid_ds buf;
	struct msginfo info;

	tst_res(TINFO, "Test MSG_STAT_ANY %s", tc->desc);

	SAFE_SETEUID(*tc->uid);

	TEST(msgctl(0, MSG_INFO, (struct msqid_ds *)&info));

	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "msgctl(0, MSG_INFO, ...)");
		return;
	}

	msgid = msgctl(TST_RET, MSG_STAT_ANY, &buf);

	if (msgid == -1) {
		tst_res(TFAIL | TERRNO, "MSG_INFO haven't returned a valid index");
	} else {
		tst_res(TPASS, "MSG_INFO returned valid index %li to msgid %i",
			TST_RET, msgid);
	}

	for (i = 0; i <= TST_RET; i++) {
		if (msgctl(i, MSG_STAT_ANY, &buf) != -1)
			cnt++;
	}

	if (cnt == info.msgpool) {
		tst_res(TPASS, "Counted used = %i", cnt);
	} else {
		tst_res(TFAIL, "Counted used = %i, msgpool = %i",
			cnt, info.msgpool);
	}

	parse_proc_sysvipc(&info);
}

static void setup(void)
{
	struct msqid_ds temp_buf;
	struct buf {
		long type;
		char text[5];
	} msgbuf = {MSGTYPE, "abcd"};
	ltpuser = SAFE_GETPWNAM("nobody");
	nobody_uid = ltpuser->pw_uid;
	root_uid = 0;

	msg_id = SAFE_MSGGET(IPC_PRIVATE, IPC_CREAT | MSG_RW);
	SAFE_MSGSND(msg_id, &msgbuf, sizeof(msgbuf.text), 0);

	TEST(msgctl(msg_id, MSG_STAT_ANY, &temp_buf));
	if (TST_RET == -1) {
		if (TST_ERR == EINVAL)
			tst_brk(TCONF, "kernel doesn't support MSG_STAT_ANY");
		else
			tst_brk(TBROK | TTERRNO,
				"Current environment doesn't permit MSG_STAT_ANY");
	}
}

static void cleanup(void)
{
	if (msg_id >= 0)
		SAFE_MSGCTL(msg_id, IPC_RMID, NULL);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_msgctl,
	.tcnt = ARRAY_SIZE(tests),
	.needs_root = 1,
};
