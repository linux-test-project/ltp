// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 *
 * HISTORY
 *	06/30/2001   Port to Linux   nsharoff@us.ibm.com
 *	10/30/2002   Port to LTP     dbarrera@us.ibm.com
 *	10/03/2008 Renaud Lottiaux (Renaud.Lottiaux@kerlabs.com)
 *	- Fix concurrency issue. A statically defined key was used. Leading
 *	  to conflict with other instances of the same test.
 */
/*\
 * Basic tests for semctl().
 *
 * - semctl() with IPC_STAT where we check the semid_ds buf content
 * - semctl() with SETVAL and GETVAL
 * - semctl() with GETPID
 * - semctl() with GETNCNT and GETZCNT
 */

#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "libnewipc.h"
#include "lapi/sem.h"

static int semid = -1;
static unsigned long nsems;

static void verify_semctl(void)
{
	int status;
	struct semid_ds buf_ds;
	union semun arg;

	arg.buf = &buf_ds;
	TST_EXP_PASS(semctl(semid, 0, IPC_STAT, arg));

	if (arg.buf->sem_nsems != nsems) {
		tst_res(TFAIL, "sem_nsems = %lu, expected %lu",
			 arg.buf->sem_nsems, nsems);
	} else {
		tst_res(TPASS, "sem_nsems = %lu", arg.buf->sem_nsems);
	}

	if (arg.buf->sem_perm.uid != getuid()) {
		tst_res(TFAIL, "sem_perm.uid = %d, expected %d",
			 arg.buf->sem_perm.uid, getuid());
	} else {
		tst_res(TPASS, "sem_perm.uid = %d", arg.buf->sem_perm.uid);
	}

	if (arg.buf->sem_perm.gid != getgid()) {
		tst_res(TFAIL, "sem_perm.gid = %d, expected %d",
			 arg.buf->sem_perm.gid, getgid());
	} else {
		tst_res(TPASS, "sem_perm.gid = %d", arg.buf->sem_perm.gid);
	}

	if (arg.buf->sem_perm.cuid != getuid()) {
		tst_res(TFAIL, "sem_perm.cuid = %d, expected %d",
			 arg.buf->sem_perm.cuid, getuid());
	} else {
		tst_res(TPASS, "sem_perm.cuid = %d", arg.buf->sem_perm.cuid);
	}

	if (arg.buf->sem_perm.cgid != getgid()) {
		tst_res(TFAIL, "sem_perm.cgid = %d, expected %d",
			 arg.buf->sem_perm.cgid, getgid());
	} else {
		tst_res(TPASS, "sem_perm.cgid = %d", arg.buf->sem_perm.cgid);
	}

	if ((status = semctl(semid, 0, GETVAL)) < 0)
		tst_res(TFAIL | TERRNO, "semctl(GETVAL)");
	else
		tst_res(TPASS, "semctl(GETVAL) succeeded");

	arg.val = 1;

	if ((status = semctl(semid, 0, SETVAL, arg)) < 0)
		tst_res(TFAIL | TERRNO, "SEMCTL(SETVAL)");
	else
		tst_res(TPASS, "semctl(SETVAL) succeeded");

	if ((status = semctl(semid, 0, GETVAL)) < 0)
		tst_res(TFAIL | TERRNO, "semctl(GETVAL)");
	else
		tst_res(TPASS, "semctl(GETVAL) succeeded");

	if (status != arg.val) {
		tst_res(TFAIL, "semctl(GETVAL) returned %d expected %d",
			status, arg.val);
	} else {
		tst_res(TPASS, "semctl(GETVAL) returned %d", status);
	}

	if ((status = semctl(semid, 0, GETPID)) < 0)
		tst_res(TFAIL | TERRNO, "semctl(GETPID)");
	else
		tst_res(TPASS, "semctl(GETPID) succeeded");

	if (status != getpid()) {
		tst_res(TFAIL, "semctl(GETPID) returned %d expected %d",
			status, getpid());
	} else {
		tst_res(TPASS, "semctl(GETPID) returned %d", status);
	}

	if ((status = semctl(semid, 0, GETNCNT)) < 0)
		tst_res(TFAIL | TERRNO, "semctl(GETNCNT)");
	else
		tst_res(TPASS, "semctl(GETNCNT) succeeded");

	if (status != 0)
		tst_res(TFAIL, "semctl(GETNCNT) returned %d expected 0",
			status);
	else
		tst_res(TPASS, "semctl(GETNCNT) returned 0");

	if ((status = semctl(semid, 0, GETZCNT)) < 0)
		tst_res(TFAIL | TERRNO, "semctl(GETZCNT)");
	else
		tst_res(TPASS, "semctl(GETZCNT) succeeded");

	if (status != 0)
		tst_res(TFAIL, "error: unexpected semzcnt %d", status);
	else
		tst_res(TPASS, "semctl(GETZCNT) succeeded 0");
}

static void setup(void)
{
	key_t key = GETIPCKEY();
	nsems = 1;

	semid = SAFE_SEMGET(key, nsems, SEM_RA | IPC_CREAT);
}

static void cleanup(void)
{
	if (semid != -1)
		SAFE_SEMCTL(semid, 0, IPC_RMID);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_semctl,
};
