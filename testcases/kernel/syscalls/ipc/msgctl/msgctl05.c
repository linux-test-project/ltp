// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 */

/*\
 * Cross verify the _high fields being set to 0 by the kernel.
 */
#include <sys/msg.h>
#include "lapi/msgbuf.h"
#include "libnewipc.h"
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"

#ifdef HAVE_MSQID64_DS_TIME_HIGH

static void run(void)
{
	struct msqid64_ds buf_ds = {
		.msg_stime_high = 0x0A0A,
		.msg_rtime_high = 0x0A0A,
		.msg_ctime_high = 0x0A0A,
	};
	int msqid;
	key_t key;

	key = GETIPCKEY();

	msqid = SAFE_MSGGET(key, IPC_CREAT | IPC_EXCL | MSG_RW | 0600);

	TEST(msgctl(msqid, IPC_STAT, (struct msqid_ds *)&buf_ds));
	if (TST_RET == -1)
		tst_brk(TFAIL | TTERRNO, "msqctl() failed");

	if (buf_ds.msg_stime_high || buf_ds.msg_rtime_high || buf_ds.msg_ctime_high)
		tst_res(TFAIL, "time_high fields aren't cleared by the kernel");
	else
		tst_res(TPASS, "time_high fields cleared by the kernel");

	SAFE_MSGCTL(msqid, IPC_RMID, NULL);
}

static struct tst_test test = {
	.test_all = run,
	.needs_tmpdir = 1,
};
#else
TST_TEST_TCONF("test requires struct msqid64_ds to have the time_high fields");
#endif
