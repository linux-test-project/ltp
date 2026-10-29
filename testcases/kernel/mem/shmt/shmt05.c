// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 *	12/20/2002	Port to LTP	robbiew@us.ibm.com
 *	06/30/2001	Port to Linux	nsharoff@us.ibm.com
 * Copyright (c) 2025 SUSE LLC Ricardo B. Marlière <rbm@suse.com>
 */

/*\
 * Create two shared memory segments and attach them to the same process
 * at two different addresses. The addresses DO BUMP into each other.
 * The second attach should Fail.
 */

#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "tse_newipc.h"
#include <sys/shm.h>

#define SHMSIZE 16

static void run(void)
{
	int shmid, shmid1;
	char *cp;
	key_t key[2];

	key[0] = GETIPCKEY();
	key[1] = GETIPCKEY();

	shmid = SAFE_SHMGET(key[0], SHMSIZE, IPC_CREAT | 0666);
	cp = SAFE_SHMAT(shmid, NULL, 0);

	shmid1 = SAFE_SHMGET(key[1], SHMSIZE, IPC_CREAT | 0666);
	TST_EXP_FAIL((long)shmat(shmid, cp + (SHMSIZE / 2), 0), EINVAL);

	SAFE_SHMCTL(shmid, IPC_RMID, NULL);
	SAFE_SHMCTL(shmid1, IPC_RMID, NULL);
}

static struct tst_test test = {
	.test_all = run,
};
