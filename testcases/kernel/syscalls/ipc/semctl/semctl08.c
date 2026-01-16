// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 */

/*\
 * Cross verify the _high fields being set to 0 by the kernel.
 */

#include "lapi/sembuf.h"
#include "lapi/sem.h"
#include "tst_test.h"
#include "tse_newipc.h"

#ifdef HAVE_SEMID64_DS_TIME_HIGH

static void run(void)
{
	struct semid64_ds buf_ds = {
		.sem_otime_high = 0x0A0A,
		.sem_ctime_high = 0x0A0A,
	};
	int semid;
	union semun arg;
	key_t key;

	/* get an IPC resource key */
	key = GETIPCKEY();

	semid = semget(key, 1, SEM_RA | IPC_CREAT);
	if (semid == -1)
		tst_brk(TBROK | TERRNO, "couldn't create semaphore");

	arg.buf = (struct semid_ds *)&buf_ds;
	TEST(semctl(semid, 0, IPC_STAT, arg));
	if (TST_RET == -1)
		tst_brk(TFAIL | TTERRNO, "semctl() failed");

	if (buf_ds.sem_otime_high || buf_ds.sem_ctime_high)
		tst_res(TFAIL, "time_high fields aren't cleared by the kernel");
	else
		tst_res(TPASS, "time_high fields cleared by the kernel");

	if (semctl(semid, 0, IPC_RMID, arg) == -1)
		tst_res(TINFO, "WARNING: semaphore deletion failed.");
}

static struct tst_test test = {
	.test_all = run,
	.needs_tmpdir = 1,
};
#else
TST_TEST_TCONF("test requires struct semid64_ds to have the time_high fields");
#endif
