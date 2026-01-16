// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 *	12/20/2002	Port to LTP	robbiew@us.ibm.com
 *	06/30/2001	Port to Linux	nsharoff@us.ibm.com
 * Copyright (c) 2025 SUSE LLC Ricardo B. Marlière <rbm@suse.com>
 */

/*\
 * Create and attach a shared memory segment, write to it
 * and then fork a child. The child verifies that the shared memory segment
 * that it inherited from the parent contains the same data that was originally
 * written to it by the parent.
 */

#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "tst_rand_data.h"
#include "tse_newipc.h"

#define SHMSIZE 16

#include "tst_test.h"

static void run(void)
{
	char *cp;
	int shmid;
	key_t key;

	key = GETIPCKEY();

	shmid = SAFE_SHMGET(key, SHMSIZE, IPC_CREAT | 0666);
	cp = SAFE_SHMAT(shmid, NULL, 0);
	memcpy(cp, tst_rand_data, SHMSIZE);

	if (!SAFE_FORK()) {
		TST_EXP_EQ_LI(memcmp(cp, tst_rand_data, SHMSIZE), 0);
		_exit(0);
	}

	SAFE_WAIT(NULL);
	SAFE_SHMCTL(shmid, IPC_RMID, NULL);
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
};
