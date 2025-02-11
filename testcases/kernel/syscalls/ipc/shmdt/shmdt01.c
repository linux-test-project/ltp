// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 */

/*\
 * This case check whether we get SIGSEGV when write a value to a detached
 * shared memory resource.
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <setjmp.h>
#include "tst_test.h"
#include "libnewipc.h"
#include "tst_safe_sysv_ipc.h"

static int shm_id = -1, shm_key, pass;
static int *shared;
static sigjmp_buf env;

static void sighandler(int sig LTP_ATTRIBUTE_UNUSED)
{
	pass = 1;
	siglongjmp(env, 1);
}

static void check_functionality(void)
{
	struct shmid_ds buf;

	SAFE_SHMCTL(shm_id, IPC_STAT, &buf);
	TST_EXP_EQ_LI(buf.shm_nattch, 0);
	if (buf.shm_nattch)
		return;

	if (sigsetjmp(env, 1) == 0)
		*shared = 2;

	if (pass)
		tst_res(TPASS, "shared memory detached correctly");
	else
		tst_res(TFAIL, "shared memory was not detached correctly");
}

static void verify_shmdt(void)
{
	TST_EXP_PASS_SILENT(shmdt(shared));
	if (TST_PASS) {
		check_functionality();
		shared = SAFE_SHMAT(shm_id, 0, 0);
	}

	pass = 0;
}

static void setup(void)
{
	shm_key = GETIPCKEY();
	shm_id = SAFE_SHMGET(shm_key, INT_SIZE, SHM_RW | IPC_CREAT | IPC_EXCL);

	SAFE_SIGNAL(SIGSEGV, sighandler);
	shared = SAFE_SHMAT(shm_id, 0, 0);
}

static void cleanup(void)
{
	if (shm_id != -1)
		SAFE_SHMCTL(shm_id, IPC_RMID, NULL);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_shmdt,
};
