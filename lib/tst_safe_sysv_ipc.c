// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Xiao yang <yangx.jy@cn.fujitsu.com>
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"

/*
 * The IPC_STAT, IPC_SET and IPC_RMID can return either 0 or -1.
 *
 * Linux specific cmds either returns -1 on failure or positive integer
 * either index into an kernel array or shared primitive indentifier.
 */
static int ret_check(int cmd, int ret)
{
	switch (cmd) {
	case IPC_STAT:
	case IPC_SET:
	case IPC_RMID:
		return ret != 0;
	default:
		return ret == -1;
	}
}

int safe_msgget(const char *file, const int lineno, key_t key, int msgflg)
{
	int rval;

	rval = msgget(key, msgflg);
	if (rval == -1) {
		tst_brk(TBROK | TERRNO, "%s:%d: msgget(%i, %x) failed",
			file, lineno, (int)key, msgflg);
	}

	return rval;
}

int safe_msgsnd(const char *file, const int lineno, int msqid, const void *msgp,
		size_t msgsz, int msgflg)
{
	int rval;

	rval = msgsnd(msqid, msgp, msgsz, msgflg);
	if (rval == -1) {
		tst_brk(TBROK | TERRNO,
			"%s:%d: msgsnd(%i, %p, %zu, %x) failed",
			file, lineno, msqid, msgp, msgsz, msgflg);
	}

	return rval;
}

ssize_t safe_msgrcv(const char *file, const int lineno, int msqid, void *msgp,
		size_t msgsz, long msgtyp, int msgflg)
{
	ssize_t rval;

	rval = msgrcv(msqid, msgp, msgsz, msgtyp, msgflg);
	if (rval == -1) {
		tst_brk(TBROK | TERRNO,
			"%s:%d: msgrcv(%i, %p, %zu, %li, %x) failed",
			file, lineno, msqid, msgp, msgsz, msgtyp, msgflg);
	}

	return rval;
}

int safe_msgctl(const char *file, const int lineno, int msqid, int cmd,
		struct msqid_ds *buf)
{
	int rval;

	rval = msgctl(msqid, cmd, buf);
	if (ret_check(cmd, rval)) {
		tst_brk(TBROK | TERRNO,
			"%s:%d: msgctl(%i, %i, %p) = %i failed",
			file, lineno, msqid, cmd, buf, rval);
	}


	return rval;
}

int safe_shmget(const char *file, const int lineno, key_t key, size_t size,
		int shmflg)
{
	int rval;

	rval = shmget(key, size, shmflg);
	if (rval == -1) {
		tst_brk(TBROK | TERRNO, "%s:%d: shmget(%i, %zu, %x) failed",
			file, lineno, (int)key, size, shmflg);
	}

	return rval;
}

void *safe_shmat(const char *file, const int lineno, int shmid,
		const void *shmaddr, int shmflg)
{
	void *rval;

	rval = shmat(shmid, shmaddr, shmflg);
	if (rval == (void *)-1) {
		tst_brk(TBROK | TERRNO, "%s:%d: shmat(%i, %p, %x) failed",
			file, lineno, shmid, shmaddr, shmflg);
	}

	return rval;
}

int safe_shmdt(const char *file, const int lineno, const void *shmaddr)
{
	int rval;

	rval = shmdt(shmaddr);
	if (rval == -1) {
		tst_brk(TBROK | TERRNO, "%s:%d: shmdt(%p) failed",
			file, lineno, shmaddr);
	}

	return rval;
}

int safe_shmctl(const char *file, const int lineno, int shmid, int cmd,
		struct shmid_ds *buf)
{
	int rval;

	rval = shmctl(shmid, cmd, buf);
	if (ret_check(cmd, rval)) {
		tst_brk(TBROK | TERRNO,
			"%s:%d: shmctl(%i, %i, %p) = %i failed",
			file, lineno, shmid, cmd, buf, rval);
	}

	return rval;
}
