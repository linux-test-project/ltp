// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 *	12/20/2002	Port to LTP	robbiew@us.ibm.com
 *	06/30/2001	Port to Linux	nsharoff@us.ibm.com
 * Copyright (c) 2025 SUSE LLC Ricardo B. Marlière <rbm@suse.com>
 */

/*\
 * Create a shared memory segment. Attach it twice at an address
 * that is provided by the system. Detach the previously attached
 * segments from the process.
 */

#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "libnewipc.h"

#define SHMSIZE 16

static void run(void)
{
	char *cp, *cp1;
	int shmid;
	key_t key;

	key = GETIPCKEY();

	shmid = SAFE_SHMGET(key, SHMSIZE, IPC_CREAT | 0666);
	cp = SAFE_SHMAT(shmid, NULL, 0);
	cp1 = SAFE_SHMAT(shmid, NULL, 0);

	SAFE_SHMDT(cp);
	SAFE_SHMDT(cp1);
	SAFE_SHMCTL(shmid, IPC_RMID, NULL);

	tst_res(TPASS, "Attached and detached twice");
}

static struct tst_test test = {
	.test_all = run,
};
