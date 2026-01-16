// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*\
 * It is a basic test for msg_next_id.
 * When the message queue identifier that msg_next_id stored is already in use,
 * call msgget with different key just use another unused value in range
 * [0,INT_MAX]. Kernel doesn't guarantee the desired id.
 */

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "tse_newipc.h"

#define NEXT_ID_PATH "/proc/sys/kernel/msg_next_id"

static int queue_id[2], pid;
static key_t msgkey[2];

static void verify_msgget(void)
{
	SAFE_FILE_PRINTF(NEXT_ID_PATH, "%d", queue_id[0]);

	queue_id[1] = SAFE_MSGGET(msgkey[1], IPC_CREAT | MSG_RW);
	if (queue_id[1] == queue_id[0])
		tst_res(TFAIL, "msg id %d has existed, msgget() returns the"
			" same msg id unexpectedly", queue_id[0]);
	else
		tst_res(TPASS, "msg id %d has existed, msgget() returns the"
			" new msgid %d", queue_id[0], queue_id[1]);

	SAFE_MSGCTL(queue_id[1], IPC_RMID, NULL);
}

static void setup(void)
{
	msgkey[0] = GETIPCKEY();
	msgkey[1] = GETIPCKEY();
	pid = getpid();
	SAFE_FILE_PRINTF(NEXT_ID_PATH, "%d", pid);
	queue_id[0] = SAFE_MSGGET(msgkey[0], IPC_CREAT | MSG_RW);
}

static void cleanup(void)
{
	int i;

	for (i = 0; i < 2; i++) {
		if (queue_id[i] != -1)
			SAFE_MSGCTL(queue_id[i], IPC_RMID, NULL);
	}
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_msgget,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_CHECKPOINT_RESTORE=y",
		NULL
	},
	.needs_root = 1,
};
