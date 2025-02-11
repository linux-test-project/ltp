// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 */

/*\
 * Cross verify the _high fields being set to 0 by the kernel.
 */

#include <sys/shm.h>
#include "lapi/shmbuf.h"
#include "libnewipc.h"
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"

#ifdef HAVE_SHMID64_DS_TIME_HIGH

static void run(void)
{
	struct shmid64_ds buf_ds = {
		.shm_atime_high = 0x0A0A,
		.shm_dtime_high = 0x0A0A,
		.shm_ctime_high = 0x0A0A,
	};
	int shmid;
	key_t key;

	/* get an IPC resource key */
	key = GETIPCKEY();

	shmid = shmget(key, SHM_SIZE, IPC_CREAT | IPC_EXCL | SHM_RW);
	if (shmid == -1)
		tst_brk(TBROK | TERRNO, "couldn't create shared memory segment");

	TEST(shmctl(shmid, IPC_STAT, (struct shmid_ds *)&buf_ds));
	if (TST_RET == -1)
		tst_brk(TFAIL | TTERRNO, "shmctl() failed");

	if (buf_ds.shm_atime_high || buf_ds.shm_dtime_high || buf_ds.shm_ctime_high)
		tst_res(TFAIL, "time_high fields aren't cleared by the kernel");
	else
		tst_res(TPASS, "time_high fields cleared by the kernel");

	SAFE_SHMCTL(shmid, IPC_RMID, NULL);
}

static struct tst_test test = {
	.test_all = run,
	.needs_tmpdir = 1,
};
#else
TST_TEST_TCONF("test requires struct shmid64_ds to have the time_high fields");
#endif
