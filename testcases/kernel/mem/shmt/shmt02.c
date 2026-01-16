// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 *	12/20/2002	Port to LTP	robbiew@us.ibm.com
 *	06/30/2001	Port to Linux	nsharoff@us.ibm.com
 * Copyright (c) 2025 SUSE LLC Ricardo B. Marlière <rbm@suse.com>
 */

/*\
 * Create and attach a shared memory segment, write to it
 * and then remove it. Verify that the shared memory segment
 * is accessible as long as the process is still alive.
 */

#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "tst_rand_data.h"
#include "tse_newipc.h"

#define SHMSIZE 16

static void run(void)
{
	char *cp;
	int shmid;
	key_t key;

	key = GETIPCKEY();

	if (!SAFE_FORK()) {
		shmid = SAFE_SHMGET(key, SHMSIZE, IPC_CREAT | 0666);

		cp = SAFE_SHMAT(shmid, NULL, 0);
		memcpy(cp, tst_rand_data, SHMSIZE);
		SAFE_SHMCTL(shmid, IPC_RMID, NULL);
		TST_EXP_EQ_LI(memcmp(cp, tst_rand_data, SHMSIZE), 0);

		TST_CHECKPOINT_WAKE(0);

		_exit(0);
	}

	TST_CHECKPOINT_WAIT(0);
	tst_reap_children();

	shmid = SAFE_SHMGET(key, SHMSIZE, IPC_CREAT | 0666);
	cp = SAFE_SHMAT(shmid, NULL, 0);
	TST_EXP_EXPR(memcmp(cp, tst_rand_data, SHMSIZE) < 0);

	SAFE_SHMCTL(shmid, IPC_RMID, NULL);
}

static struct tst_test test = {
	.test_all = run,
	.needs_checkpoints = 1,
	.forks_child = 1,
};
