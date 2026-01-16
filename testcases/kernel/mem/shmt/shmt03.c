// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 *	12/20/2002	Port to LTP	robbiew@us.ibm.com
 *	06/30/2001	Port to Linux	nsharoff@us.ibm.com
 * Copyright (c) 2025 SUSE LLC Ricardo B. Marlière <rbm@suse.com>
 */

/*\
 * Create one shared memory segment and attach it twice to the same process,
 * at an address that is chosen by the system. After the first attach has
 * completed, write to it and then do the second attach.
 * Verify that the doubly attached segment contains the same data.
 */

#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "tst_rand_data.h"
#include "tse_newipc.h"

#define SHMSIZE 16

static void run(void)
{
	char *cp1, *cp2;
	int shmid;
	key_t key;

	key = GETIPCKEY();

	shmid = SAFE_SHMGET(key, SHMSIZE, IPC_CREAT | 0666);

	cp1 = SAFE_SHMAT(shmid, NULL, 0);
	memcpy(cp1, tst_rand_data, SHMSIZE);

	cp2 = SAFE_SHMAT(shmid, NULL, 0);
	TST_EXP_EQ_LI(memcmp(cp2, tst_rand_data, SHMSIZE), 0);

	SAFE_SHMCTL(shmid, IPC_RMID, NULL);
}

static struct tst_test test = {
	.test_all = run,
};
