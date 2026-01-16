// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015   Author: Gabriellla Schmidt <gsc@bruker.de>
 *                      Modify: Li Wang <liwang@redhat.com>
 * A regression test for:
 *      commit e7ca2552369c1dfe0216c626baf82c3d83ec36bb
 *      Author: Mateusz Guzik <mguzik@redhat.com>
 *      Date:   Mon Jan 27 17:07:11 2014 -0800
 *
 *           ipc: fix compat msgrcv with negative msgtyp
 *
 * Reproduce:
 *
 *      32-bit application using the msgrcv() system call
 *      gives the error message:
 *
 *           msgrcv: No message of desired type
 *
 *      If this progarm is compiled as 64-bit application it works.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "tse_newipc.h"

static long mtype = 121;
static key_t msgkey;
static int queue_id = -1;
static struct mbuf {
	long mtype;
	char mtext[16];
} rcv_buf, snd_buf = {121, "hello"};

static void verify_msgrcv(void)
{
	memset(&rcv_buf, 0, sizeof(rcv_buf));
	SAFE_MSGSND(queue_id, &snd_buf, sizeof(snd_buf.mtext), IPC_NOWAIT);

	TEST(msgrcv(queue_id, &rcv_buf, sizeof(rcv_buf.mtext), -mtype, IPC_NOWAIT | MSG_NOERROR));
	if (TST_RET == -1) {
		tst_res(TFAIL, "Bug: No message of desired type.");
		return;
	}

	if (rcv_buf.mtype != mtype) {
		tst_res(TFAIL, "found mtype %ld, expected %ld", rcv_buf.mtype, mtype);
		return;
	}
	if ((size_t)TST_RET != sizeof(snd_buf.mtext)) {
		tst_res(TFAIL, "received %zi, expected %zu", (size_t)TST_RET, sizeof(snd_buf.mtext));
		return;
	}

	tst_res(TPASS, "No regression found.");
}

static void setup(void)
{
	msgkey = GETIPCKEY();
	queue_id = SAFE_MSGGET(msgkey, IPC_CREAT | IPC_EXCL | MSG_RW);
}

static void cleanup(void)
{
	if (queue_id != -1)
		SAFE_MSGCTL(queue_id, IPC_RMID, NULL);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_msgrcv,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "e7ca2552369c"},
		{}
	}
};
