// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2020 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Call shmctl() with IPC_INFO flag and check that the data are consistent with
 * /proc/sys/kernel/shm*.
 */

#define _GNU_SOURCE
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "tse_newipc.h"

static void verify_ipcinfo(void)
{
	struct shminfo info;

	TEST(shmctl(0, IPC_INFO, (struct shmid_ds *)&info));

	if (TST_RET < 0) {
		tst_res(TFAIL | TTERRNO,
			"shmctl(0, IPC_INFO, ...) returned %li", TST_RET);
		return;
	}

	if (info.shmmin != 1)
		tst_res(TFAIL, "shmmin = %li, expected 1", info.shmmin);
	else
		tst_res(TPASS, "shmmin = 1");

	TST_ASSERT_ULONG("/proc/sys/kernel/shmmax", info.shmmax);
	TST_ASSERT_ULONG("/proc/sys/kernel/shmmni", info.shmmni);
	TST_ASSERT_ULONG("/proc/sys/kernel/shmall", info.shmall);
}

static struct tst_test test = {
	.test_all = verify_ipcinfo,
};
