// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  03/2001 - Written by Wayne Boyer
 *
 * Copyright (c) 2008 Renaud Lottiaux (Renaud.Lottiaux@kerlabs.com)
 *
 * Copyright (C) 2020 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Test for EACCES, EFAULT and EINVAL errors.
 *
 * * EACCES - segment has no read or write permissions
 * * EFAULT - IPC_SET & buf isn't valid
 * * EFAULT - IPC_STAT & buf isn't valid
 * * EINVAL - the command is not valid
 * * EINVAL - the shmid is not valid
 * * EINVAL - the shmid belongs to removed shm
 *
 * * EACCES - attempt to stat root-owned shm
 * * EPERM  - attempt to delete root-owned shm
 * * EPERM  - attempt to change root-owned shm
 * * EPERM  - attempt to lock root-owned shm
 * * EPERM  - attempt to unlock root-owned shm
 */

#include <pwd.h>

#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "libnewipc.h"

#define SHM_SIZE 2048

static int shm_id1 = -1;
static int shm_id2 = -1;
static int shm_id3 = -1;
static int shm_bad = -1;
static int shm_rem;

static struct shmid_ds buf;

static struct tcase {
	int *shm_id;
	int cmd;
	struct shmid_ds *buf;
	int error;
} tc[] = {
	{&shm_id1, IPC_STAT, &buf, EACCES},
	{&shm_id2, IPC_SET, (struct shmid_ds *)-1, EFAULT},
	{&shm_id2, IPC_STAT, (struct shmid_ds *)-1, EFAULT},
	{&shm_id2, -1, &buf, EINVAL},
	{&shm_bad, IPC_STAT, &buf, EINVAL},
	{&shm_rem, IPC_STAT, &buf, EINVAL},
	/* Operations on root-owned shm */
	{&shm_id3, IPC_STAT, &buf, EACCES},
	{&shm_id3, IPC_RMID, NULL, EPERM},
	{&shm_id3, IPC_SET, &buf, EPERM},
	{&shm_id3, SHM_LOCK, &buf, EPERM},
	{&shm_id3, SHM_UNLOCK, &buf, EPERM}
};

static void verify_shmctl(unsigned int i)
{
	TEST(shmctl(*(tc[i].shm_id), tc[i].cmd, tc[i].buf));

	if (TST_RET != -1) {
		tst_res(TFAIL, "shmctl() returned %li", TST_RET);
		return;
	}

	if (TST_ERR == tc[i].error) {
		tst_res(TPASS | TTERRNO, "shmctl(%i, %i, %p)",
		        *tc[i].shm_id, tc[i].cmd, tc[i].buf);
		return;
	}

	tst_res(TFAIL | TTERRNO, "shmctl(%i, %i, %p) expected %s",
		*tc[i].shm_id, tc[i].cmd, tc[i].buf, tst_strerrno(tc[i].error));
}

static void setup(void)
{
	key_t shmkey1, shmkey2;
	struct passwd *ltpuser;
	int tmp;

	shm_id3 = SAFE_SHMGET(IPC_PRIVATE, SHM_SIZE, IPC_CREAT | SHM_RW);

	ltpuser = SAFE_GETPWNAM("nobody");
	SAFE_SETEUID(ltpuser->pw_uid);

	shmkey1 = GETIPCKEY();
	shmkey2 = GETIPCKEY();

	shm_id1 = SAFE_SHMGET(shmkey1, SHM_SIZE, IPC_CREAT | IPC_EXCL);
	shm_id2 = SAFE_SHMGET(shmkey2, SHM_SIZE, IPC_CREAT | IPC_EXCL | SHM_RW);

	tmp = shm_rem = SAFE_SHMGET(IPC_PRIVATE, SHM_SIZE, IPC_CREAT | SHM_RW);
	SAFE_SHMCTL(tmp, IPC_RMID, NULL);
}

static void cleanup(void)
{
	if (shm_id1 >= 0)
		SAFE_SHMCTL(shm_id1, IPC_RMID, NULL);

	if (shm_id2 >= 0)
		SAFE_SHMCTL(shm_id2, IPC_RMID, NULL);

	if (shm_id3 >= 0) {
		SAFE_SETEUID(0);
		SAFE_SHMCTL(shm_id3, IPC_RMID, NULL);
	}
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_shmctl,
	.tcnt = ARRAY_SIZE(tc),
	.needs_root = 1,
};
