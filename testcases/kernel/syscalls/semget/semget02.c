// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 */

/*\
 * This basic error handing of the semget syscall.
 *
 * - EACCES - a semaphore set exists for key, but the calling process does not
 *   have permission to access the set
 * - EEXIST - a semaphore set already exists for key and IPC_CREAT | IPC_EXCL
 *   is given
 * - ENOENT - No semaphore set exists for key and semflg did not specify
 *   IPC_CREAT
 * - EINVAL - nsems is less than 0 or greater than the limit on the number of
 *   semaphores per semaphore set(SEMMSL)
 * - EINVAL - a semaphore set corresponding to key already exists, but nsems is
 *   larger than the number of semaphores in that set
 */

#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <pwd.h>
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "tse_newipc.h"
#include "lapi/sem.h"

static int sem_id = -1;
static key_t semkey, semkey1;
static struct passwd *pw;
static struct tcase {
	int *key;
	int nsems;
	int flags;
	int exp_err;
	/*1: nobody expected, 0: root expected */
	int exp_user;
} tcases[] = {
	{&semkey, PSEMS, SEM_RA, EACCES, 1},
	{&semkey, PSEMS, IPC_CREAT | IPC_EXCL, EEXIST, 0},
	{&semkey1, PSEMS, SEM_RA, ENOENT, 0},
	{&semkey1, -1, IPC_CREAT | IPC_EXCL, EINVAL, 0},
	{&semkey1, SEMMSL + 1, IPC_CREAT | IPC_EXCL, EINVAL, 0},
	{&semkey, PSEMS + 1, SEM_RA, EINVAL, 0},
};

static void verify_semget(struct tcase *tc)
{
	TST_EXP_FAIL2(semget(*tc->key, tc->nsems, tc->flags), tc->exp_err,
			"semget(%i, %i, %i)", *tc->key, tc->nsems, tc->flags);
}

static void do_test(unsigned int n)
{
	pid_t pid;
	struct tcase *tc = &tcases[n];

	if (tc->exp_user == 0) {
		verify_semget(tc);
	} else {
		pid = SAFE_FORK();
		if (pid) {
			tst_reap_children();
		} else {
			SAFE_SETUID(pw->pw_uid);
			verify_semget(tc);
			exit(0);
		}
	}
}

static void setup(void)
{
	semkey = GETIPCKEY();
	semkey1 = GETIPCKEY();

	sem_id = SAFE_SEMGET(semkey, PSEMS, IPC_CREAT | IPC_EXCL);

	pw = SAFE_GETPWNAM("nobody");
}

static void cleanup(void)
{
	if (sem_id != -1)
		SAFE_SEMCTL(sem_id, PSEMS, IPC_RMID);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.needs_root = 1,
	.forks_child = 1,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.test = do_test,
};
