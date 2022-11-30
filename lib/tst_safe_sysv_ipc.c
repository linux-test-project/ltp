// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Xiao yang <yangx.jy@cn.fujitsu.com>
 */

#include <sys/types.h>
#include <sys/msg.h>
#include <sys/shm.h>
#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "lapi/ipc.h"
#include "lapi/sem.h"

/*
 * The IPC_STAT, IPC_SET, IPC_RMID can return either 0 or -1.
 */
static int msg_ret_check(int cmd, int ret)
{
	switch (cmd) {
	case IPC_STAT:
	case IPC_SET:
	case IPC_RMID:
		return ret != 0;
	default:
		return ret < 0;
	}
}

/*
 * The IPC_STAT, IPC_SET, IPC_RMID, SHM_LOCK, SHM_UNLOCK can return either 0 or -1.
 */
static int shm_ret_check(int cmd, int ret)
{
	switch (cmd) {
	case IPC_STAT:
	case IPC_SET:
	case IPC_RMID:
	case SHM_LOCK:
	case SHM_UNLOCK:
		return ret != 0;
	default:
		return ret < 0;
	}
}

/*
 * The IPC_STAT, IPC_SET, IPC_RMID, SETALL, SETVAL can return either 0 or -1.
 */
static int sem_ret_check(int cmd, int ret)
{
	switch (cmd) {
	case IPC_STAT:
	case IPC_SET:
	case IPC_RMID:
	case SETALL:
	case SETVAL:
		return ret != 0;
	default:
		return ret < 0;
	}
}

int safe_msgget(const char *file, const int lineno, key_t key, int msgflg)
{
	int rval;

	rval = msgget(key, msgflg);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO, "msgget(%i, %x) failed",
			(int)key, msgflg);
	} else if (rval < 0) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid msgget(%i, %x) return value %d", (int)key,
			msgflg, rval);
	}

	return rval;
}

int safe_msgsnd(const char *file, const int lineno, int msqid, const void *msgp,
		size_t msgsz, int msgflg)
{
	int rval;

	rval = msgsnd(msqid, msgp, msgsz, msgflg);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"msgsnd(%i, %p, %zu, %x) failed", msqid, msgp, msgsz,
			msgflg);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid msgsnd(%i, %p, %zu, %x) return value %d",
			msqid, msgp, msgsz, msgflg, rval);
	}

	return rval;
}

ssize_t safe_msgrcv(const char *file, const int lineno, int msqid, void *msgp,
		size_t msgsz, long msgtyp, int msgflg)
{
	ssize_t rval;

	rval = msgrcv(msqid, msgp, msgsz, msgtyp, msgflg);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"msgrcv(%i, %p, %zu, %li, %x) failed",
			msqid, msgp, msgsz, msgtyp, msgflg);
	} else if (rval < 0) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid msgrcv(%i, %p, %zu, %li, %x) return value %ld",
			msqid, msgp, msgsz, msgtyp, msgflg, rval);
	}

	return rval;
}

int safe_msgctl(const char *file, const int lineno, int msqid, int cmd,
		struct msqid_ds *buf)
{
	int rval;

	rval = msgctl(msqid, cmd, buf);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"msgctl(%i, %i, %p) failed", msqid, cmd, buf);
	} else if (msg_ret_check(cmd, rval)) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid msgctl(%i, %i, %p) return value %d", msqid,
			cmd, buf, rval);
	}

	return rval;
}

int safe_shmget(const char *file, const int lineno, key_t key, size_t size,
		int shmflg)
{
	int rval;

	rval = shmget(key, size, shmflg);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"shmget(%i, %zu, %x) failed", (int)key, size, shmflg);
	} else if (rval < 0) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid shmget(%i, %zu, %x) return value %d",
			(int)key, size, shmflg, rval);
	}

	return rval;
}

void *safe_shmat(const char *file, const int lineno, int shmid,
		const void *shmaddr, int shmflg)
{
	void *rval;

	rval = shmat(shmid, shmaddr, shmflg);

	if (rval == (void *)-1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"shmat(%i, %p, %x) failed", shmid, shmaddr, shmflg);
	}

	return rval;
}

int safe_shmdt(const char *file, const int lineno, const void *shmaddr)
{
	int rval;

	rval = shmdt(shmaddr);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO, "shmdt(%p) failed",
			shmaddr);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid shmdt(%p) return value %d", shmaddr, rval);
	}

	return rval;
}

int safe_shmctl(const char *file, const int lineno, int shmid, int cmd,
		struct shmid_ds *buf)
{
	int rval;

	rval = shmctl(shmid, cmd, buf);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"shmctl(%i, %i, %p) failed", shmid, cmd, buf);
	} else if (shm_ret_check(cmd, rval)) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid shmctl(%i, %i, %p) return value %d", shmid,
			cmd, buf, rval);
	}

	return rval;
}

int safe_semget(const char *file, const int lineno, key_t key, int nsems,
		int semflg)
{
	int rval;

	rval = semget(key, nsems, semflg);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"semget(%i, %i, %x) failed", (int)key, nsems, semflg);
	} else if (rval < 0) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid semget(%i, %i, %x) return value %d",
			(int)key, nsems, semflg, rval);
	}

	return rval;
}

int safe_semctl(const char *file, const int lineno, int semid, int semnum,
		int cmd, ...)
{
	int rval;
	va_list va;
	union semun un = {0};

	switch (cmd) {
	case SETVAL:
	case GETALL:
	case SETALL:
	case IPC_STAT:
	case IPC_SET:
	case SEM_STAT:
	case SEM_STAT_ANY:
	case IPC_INFO:
	case SEM_INFO:
		va_start(va, cmd);
		un = va_arg(va, union semun);
		va_end(va);
	}

	rval = semctl(semid, semnum, cmd, un);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
		"semctl(%i, %i, %i,...) failed", semid, semnum, cmd);
	} else if (sem_ret_check(cmd, rval)) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid semctl(%i, %i, %i,...) return value %d", semid,
			semnum, cmd, rval);
	}

	return rval;
}

int safe_semop(const char *file, const int lineno, int semid, struct sembuf *sops,
		size_t nsops)
{
	int rval;

	rval = semop(semid, sops, nsops);
	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"semop(%d, %p, %zu) failed", semid, sops, nsops);
	} else if (rval < 0) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid semop(%d, %p, %zu) return value %d",
			semid, sops, nsops, rval);
	}

	return rval;
}
