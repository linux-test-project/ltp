/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2017 Xiao yang <yangx.jy@cn.fujitsu.com>
 */

#ifndef TST_SAFE_SYSV_IPC_H__
#define TST_SAFE_SYSV_IPC_H__

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

int safe_msgget(const char *file, const int lineno, key_t key, int msgflg);
#define SAFE_MSGGET(key, msgflg) \
	safe_msgget(__FILE__, __LINE__, (key), (msgflg))

int safe_msgsnd(const char *file, const int lineno, int msqid, const void *msgp,
		size_t msgsz, int msgflg);
#define SAFE_MSGSND(msqid, msgp, msgsz, msgflg) \
	safe_msgsnd(__FILE__, __LINE__, (msqid), (msgp), (msgsz), (msgflg))

ssize_t safe_msgrcv(const char *file, const int lineno, int msqid, void *msgp,
		size_t msgsz, long msgtyp, int msgflg);
#define SAFE_MSGRCV(msqid, msgp, msgsz, msgtyp, msgflg) \
	safe_msgrcv(__FILE__, __LINE__, (msqid), (msgp), (msgsz), (msgtyp), (msgflg))

int safe_msgctl(const char *file, const int lineno, int msqid, int cmd,
		struct msqid_ds *buf);
#define SAFE_MSGCTL(msqid, cmd, buf) ({ \
	int tst_ret_ = safe_msgctl(__FILE__, __LINE__, (msqid), (cmd), (buf)); \
	(msqid) = ((cmd) == IPC_RMID ? -1 : (msqid)); \
	tst_ret_;})

int safe_shmget(const char *file, const int lineno, key_t key, size_t size,
		int shmflg);
#define SAFE_SHMGET(key, size, shmflg) \
	safe_shmget(__FILE__, __LINE__, (key), (size), (shmflg))

void *safe_shmat(const char *file, const int lineno, int shmid,
		const void *shmaddr, int shmflg);
#define SAFE_SHMAT(shmid, shmaddr, shmflg) \
	safe_shmat(__FILE__, __LINE__, (shmid), (shmaddr), (shmflg))

int safe_shmdt(const char *file, const int lineno, const void *shmaddr);
#define SAFE_SHMDT(shmaddr)	safe_shmdt(__FILE__, __LINE__, (shmaddr))

int safe_shmctl(const char *file, const int lineno, int shmid, int cmd,
		struct shmid_ds *buf);
#define SAFE_SHMCTL(shmid, cmd, buf) ({ \
	int tst_ret_ = safe_shmctl(__FILE__, __LINE__, (shmid), (cmd), (buf)); \
	(shmid) = ((cmd) == IPC_RMID ? -1 : (shmid)); \
	tst_ret_;})

#endif /* TST_SAFE_SYSV_IPC_H__ */
