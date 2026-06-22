// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 */
/*
 * DESCRIPTION
 *	msgctl13 - test for IPC_RMID
 */
#include <errno.h>

#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "tse_newipc.h"

static void verify_msgctl(void)
{
	struct msqid_ds buf;
	int msg_q;

	msg_q = SAFE_MSGGET(IPC_PRIVATE, MSG_RW);

	TEST(msgctl(msg_q, IPC_RMID, NULL));
	if (TST_RET != 0) {
		tst_res(TFAIL | TTERRNO, "msgctl(IPC_RMID) failed");
		return;
	}

	tst_res(TPASS, "msgctl(IPC_RMID)");

	TEST(msgctl(msg_q, IPC_STAT, &buf));
	if (TST_ERR == EINVAL) {
		tst_res(TPASS | TTERRNO, "msgctl(IPC_STAT)");
	} else {
		tst_res(TFAIL | TTERRNO,
			"msgctl(IPC_STAT) returned %li", TST_RET);
	}
}

static struct tst_test test = {
	.test_all = verify_msgctl,
	.needs_tmpdir = 1
};
