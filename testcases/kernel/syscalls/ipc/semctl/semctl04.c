// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 */
/*\
 * Test for semctl() EPERM error
 *
 * Runs IPC_SET and IPC_RMID from unprivileged child process.
 *
 */

#include <pwd.h>
#include <sys/wait.h>
#include "tst_safe_sysv_ipc.h"
#include "tst_test.h"
#include "lapi/sem.h"
#include "libnewipc.h"

static uid_t ltp_uid;
static int sem_id = -1;

static int tcases[] = { IPC_SET, IPC_RMID };

static void do_child(void)
{
	int i;
	union semun arg;
	struct semid_ds perm;

	for (i = 0; i < 2; i++) {
		if (tcases[i] == IPC_SET) {
			arg.buf = &perm;
			memset(&perm, 0, sizeof(perm));
			perm.sem_perm.uid = getuid() + 1;
			perm.sem_perm.gid = getgid() + 1;
			perm.sem_perm.mode = 0666;
		}

		TST_EXP_FAIL(semctl(sem_id, 0, tcases[i], arg), EPERM,
			     "semctl() with nobody user in child");
	}
}

static void verify_semctl(void)
{
	pid_t pid;

	pid = SAFE_FORK();

	if (pid == 0) {
		SAFE_SETEUID(ltp_uid);
		do_child();
	} else {
		SAFE_WAITPID(pid, NULL, 0);
	}
}

static void setup(void)
{
	static key_t semkey;

	semkey = GETIPCKEY();

	sem_id = SAFE_SEMGET(semkey, PSEMS, IPC_CREAT | IPC_EXCL);

	struct passwd *ltpuser = SAFE_GETPWNAM("nobody");
	ltp_uid = ltpuser->pw_uid;
}

static void cleanup(void)
{
	if (sem_id != -1)
		SAFE_SEMCTL(sem_id, 0, IPC_RMID);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.needs_root = 1,
	.test_all = verify_semctl,
};
