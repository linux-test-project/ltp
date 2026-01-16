// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  03/2001 - Written by Wayne Boyer
 */

/*\
 * Test for EACCES error.
 *
 * Create a shared memory segment without read or write permissions under
 * unpriviledged user and call shmget() with SHM_RD/SHM_WR/SHM_RW flag to
 * trigger EACCES error.
 */
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/shm.h>
#include "tst_safe_sysv_ipc.h"
#include "tst_test.h"
#include "tse_newipc.h"
#include "lapi/shm.h"

static int shm_id = -1;
static key_t shmkey;
static struct tcase {
	char *message;
	int flag;
} tcases[] = {
	{"Testing SHM_RD flag", SHM_RD},
	{"Testing SHM_WR flag", SHM_WR},
	{"Testing SHM_RW flag", SHM_RW},
};

static void verify_shmget(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	tst_res(TINFO, "%s", tc->message);
	TST_EXP_FAIL2(shmget(shmkey, SHM_SIZE, tc->flag), EACCES, "shmget(%i, %i, %i)",
		shmkey, SHM_SIZE, tc->flag);
}

static void setup(void)
{
	struct passwd *pw;

	pw = SAFE_GETPWNAM("nobody");
	SAFE_SETUID(pw->pw_uid);
	shmkey = GETIPCKEY();

	shm_id = SAFE_SHMGET(shmkey, SHM_SIZE, IPC_CREAT | IPC_EXCL);
}

static void cleanup(void)
{
	if (shm_id >= 0)
		SAFE_SHMCTL(shm_id, IPC_RMID, NULL);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_shmget,
	.tcnt = ARRAY_SIZE(tcases),
};
