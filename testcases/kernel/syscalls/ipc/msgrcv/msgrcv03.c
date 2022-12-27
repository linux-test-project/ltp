// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.jujitsu.com>
 *
 * This is a basic test about MSG_COPY flag.
 * This flag was added in 3.8 for the implementation of the kernel checkpoint
 * restore facility and is available only if the kernel was built with the
 * CONFIG_CHECKPOINT_RESTORE option.
 * On old kernel without this support, it only ignores this flag and doesn't
 * report ENOSYS/EINVAL error. The CONFIG_CHECKPOINT_RESTORE has existed
 * before kernel 3.8.
 * So for using this flag, kernel should greater than 3.8 and enable
 * CONFIG_CHECKPOINT_RESTORE together.
 *
 * 1)msgrcv(2) fails and sets errno to EINVAL if IPC_NOWAIT was not specified
 *   in msgflag.
 * 2)msgrcv(2) fails and sets errno to EINVAL if IPC_EXCEPT was specified
 *   in msgflag.
 * 3)msgrcv(2) fails and set errno to ENOMSG if IPC_NOWAIT and MSG_COPY were
 *  specified in msgflg and the queue contains less than msgtyp messages.
 */

#define  _GNU_SOURCE
#include <string.h>
#include <sys/wait.h>
#include <pwd.h>
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "libnewipc.h"
#include "lapi/msg.h"

static key_t msgkey;
static int queue_id = -1;
static struct buf {
	long type;
	char mtext[MSGSIZE];
} rcv_buf, snd_buf = {MSGTYPE, "hello"};

static struct tcase {
	int exp_err;
	int msg_flag;
	int msg_type;
	char *message;
} tcases[] = {
	{EINVAL, 0, MSGTYPE,
	"EINVAL for MSG_COPY without IPC_NOWAIT"},

	{EINVAL, MSG_EXCEPT, MSGTYPE,
	"EINVAL for MSG_COPY with MSG_EXCEPT"},

	{ENOMSG, IPC_NOWAIT, 2,
	"ENOMSG with IPC_NOWAIT and MSG_COPY but with less than msgtyp messages"},
};

static void verify_msgrcv(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	tst_res(TINFO, "%s", tc->message);

	TST_EXP_FAIL2(msgrcv(queue_id, &rcv_buf, MSGSIZE, tc->msg_type, MSG_COPY | tc->msg_flag), tc->exp_err,
		"msgrcv(%i, %p, %i, %i, %i)", queue_id, &rcv_buf, MSGSIZE, tc->msg_type, MSG_COPY | tc->msg_flag);
}

static void setup(void)
{
	msgkey = GETIPCKEY();
	queue_id = SAFE_MSGGET(msgkey, IPC_CREAT | IPC_EXCL | MSG_RW);
	SAFE_MSGSND(queue_id, &snd_buf, MSGSIZE, 0);
}

static void cleanup(void)
{
	if (queue_id != -1)
		SAFE_MSGCTL(queue_id, IPC_RMID, NULL);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.needs_root = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_CHECKPOINT_RESTORE",
		NULL
	},
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_msgrcv,
	.setup = setup,
	.cleanup = cleanup,
};
