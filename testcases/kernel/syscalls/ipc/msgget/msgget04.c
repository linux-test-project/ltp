// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*\
 * It is a basic test for msg_next_id.
 * msg_next_id specifies desired id for next allocated IPC message. By
 * default it's equal to -1, which means generic allocation logic.
 * Possible values to set are in range {0..INT_MAX}.
 * The value will be set back to -1 by kernel after successful IPC object
 * allocation.
 */

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "libnewipc.h"

#define NEXT_ID_PATH "/proc/sys/kernel/msg_next_id"
static int queue_id, pid;
static key_t msgkey;

static void verify_msgget(void)
{
	SAFE_FILE_PRINTF(NEXT_ID_PATH, "%d", pid);

	queue_id = SAFE_MSGGET(msgkey, IPC_CREAT | MSG_RW);
	if (queue_id == pid)
		tst_res(TPASS, "msg_next_id succeeded, queue id %d", pid);
	else
		tst_res(TFAIL, "msg_next_id failed, expected id %d, but got %d", pid, queue_id);

	TST_ASSERT_INT(NEXT_ID_PATH, -1);
	SAFE_MSGCTL(queue_id, IPC_RMID, NULL);
	pid++;
}

static void setup(void)
{
	msgkey = GETIPCKEY();
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
	.test_all = verify_msgget,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_CHECKPOINT_RESTORE=y",
		NULL
	},
	.needs_root = 1,
};
