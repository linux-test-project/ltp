// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 */

/*\
 * Test for ENOSPC error.
 *
 * ENOSPC -  All possible message queues have been taken (MSGMNI)
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>

#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "tse_newipc.h"

static int maxmsgs, queue_cnt, used_cnt;
static int *queues;
static key_t msgkey;

static void verify_msgget(void)
{
	TST_EXP_FAIL2(msgget(msgkey + maxmsgs, IPC_CREAT | IPC_EXCL), ENOSPC,
		"msgget(%i, %i)", msgkey + maxmsgs, IPC_CREAT | IPC_EXCL);
}

static void setup(void)
{
	int res, num;

	msgkey = GETIPCKEY();

	used_cnt = GET_USED_QUEUES();
	tst_res(TINFO, "Current environment %d message queues are already in use",
		used_cnt);

	maxmsgs = used_cnt + 32;
	SAFE_FILE_PRINTF("/proc/sys/kernel/msgmni", "%i", maxmsgs);

	queues = SAFE_MALLOC((maxmsgs - used_cnt) * sizeof(int));

	for (num = 0; num < maxmsgs - used_cnt; num++) {
		res = msgget(msgkey + num, IPC_CREAT | IPC_EXCL);
		if (res == -1)
			tst_brk(TBROK | TERRNO, "msgget failed unexpectedly");
		queues[queue_cnt++] = res;
	}

	tst_res(TINFO, "The maximum number of message queues (%d) reached",
		maxmsgs);
}

static void cleanup(void)
{
	int num;

	if (!queues)
		return;

	for (num = 0; num < queue_cnt; num++)
		SAFE_MSGCTL(queues[num], IPC_RMID, NULL);

	free(queues);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_msgget,
	.save_restore = (const struct tst_path_val[]){
		{"/proc/sys/kernel/msgmni", NULL, TST_SR_TCONF},
		{}
	}
};
