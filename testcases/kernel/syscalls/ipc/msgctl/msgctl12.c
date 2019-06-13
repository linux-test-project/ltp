// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 */
/*
 * msgctl12 - test for IPC_INFO MSG_INFO and MSG_STAT.
 */

#define _GNU_SOURCE
#include <errno.h>

#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "libnewipc.h"

static int msg_q = -1;
static int index_q;
static struct msginfo msginfo_buf;
static struct msqid_ds msgqid_buf;

static struct tcase {
	int *msg_id;
	int cmd;
	char *name;
	void *buf;
} tc[] = {
	{&msg_q, IPC_INFO, "IPC_INFO", &msginfo_buf},
	{&msg_q, MSG_INFO, "MSG_INFO", &msginfo_buf},
	{&index_q, MSG_STAT, "MSG_STAT", &msgqid_buf},
};

static void verify_msgctl(unsigned int i)
{
	TEST(msgctl(*tc[i].msg_id,  tc[i].cmd, tc[i].buf));

	if (TST_RET == -1) {
		tst_res(TFAIL,
			 "msgctl() test %s failed with errno: "
			 "%d", tc[i].name, TST_ERR);
	}

	tst_res(TPASS, "msgctl() test %s succeeded", tc[i].name);
}

static void setup(void)
{
	msg_q = SAFE_MSGGET(IPC_PRIVATE, MSG_RW);
	index_q = SAFE_MSGCTL(msg_q, IPC_INFO, (struct msqid_ds*)&msginfo_buf);
}

static void cleanup(void)
{
	if (msg_q >= 0)
		SAFE_MSGCTL(msg_q, IPC_RMID, NULL);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_msgctl,
	.tcnt = ARRAY_SIZE(tc),
};
