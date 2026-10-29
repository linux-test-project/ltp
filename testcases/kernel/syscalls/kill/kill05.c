// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 *      26/02/2008 Renaud Lottiaux (Renaud.Lottiaux@kerlabs.com)
 *      - Fix wrong return value check on shmat system call (leading to
 *        segfault in case of error with this syscall).
 *      - Fix deletion of IPC memory segment. Segment was not correctly
 *        deleted due to the change of uid during the test.
 */

/*\
 * Test case to check that kill() fails when passed a pid owned by another user.
 */

#include <sys/wait.h>
#include <pwd.h>
#include <stdlib.h>
#include "tst_test.h"
#include "tse_newipc.h"
#include "tst_safe_sysv_ipc.h"
#include "tst_safe_macros.h"
#include "tst_uid.h"

static uid_t test_users[2];
static int *flag;
static int shm_id = -1;
static key_t shm_key;

static void wait_for_flag(int value)
{
	while (1) {
		if (*flag == value)
			break;

		usleep(100);
	}
}

static void do_master_child(void)
{
	pid_t pid1;

	*flag = 0;
	pid1 = SAFE_FORK();
	if (pid1 == 0) {
		SAFE_SETREUID(test_users[0], test_users[0]);
		*flag = 1;
		wait_for_flag(2);

		exit(0);
	}

	SAFE_SETREUID(test_users[1], test_users[1]);
	wait_for_flag(1);
	TEST(kill(pid1, SIGKILL));

	*flag = 2;
	SAFE_WAITPID(pid1, NULL, 0);

	if (TST_RET == 0)
		tst_brk(TFAIL, "kill succeeded unexpectedly");

	if (TST_ERR == EPERM)
		tst_res(TPASS, "kill failed with EPERM");
	else
		tst_res(TFAIL | TTERRNO, "kill failed expected EPERM, but got");
}

static void verify_kill(void)
{
	pid_t pid;

	pid = SAFE_FORK();
	if (pid == 0) {
		do_master_child();
		exit(0);
	}

	tst_reap_children();
}

static void setup(void)
{
	shm_key = GETIPCKEY();
	shm_id = SAFE_SHMGET(shm_key, getpagesize(), 0666 | IPC_CREAT);
	flag = SAFE_SHMAT(shm_id, 0, 0);
	tst_get_uids(test_users, 0, 2);
}

static void cleanup(void)
{
	if (shm_id != -1)
		SAFE_SHMCTL(shm_id, IPC_RMID, NULL);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_kill,
	.needs_root = 1,
	.forks_child = 1,
};
