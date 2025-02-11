// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *
 *	03/2001 - Written by Wayne Boyer
 */
/*\
 * Test for semctl() EACCES error.
 */

#include <pwd.h>
#include "tst_safe_sysv_ipc.h"
#include "tst_test.h"
#include "lapi/sem.h"
#include "libnewipc.h"

static int sem_id = -1;

static void verify_semctl(void)
{
	struct semid_ds sem_ds;
	union semun un_arg;

	un_arg.buf = &sem_ds;

	TST_EXP_FAIL(semctl(sem_id, 0, IPC_STAT, un_arg), EACCES,
		     "semctl(IPC_STAT) with nobody user");
}

static void setup(void)
{
	static key_t semkey;
	struct passwd *ltpuser = SAFE_GETPWNAM("nobody");

	SAFE_SETUID(ltpuser->pw_uid);

	semkey = GETIPCKEY();

	sem_id = SAFE_SEMGET(semkey, PSEMS, IPC_CREAT | IPC_EXCL);
}

void cleanup(void)
{
	if (sem_id != -1)
		SAFE_SEMCTL(sem_id, 0, IPC_RMID);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.test_all = verify_semctl,
};
