/*
 * Copyright (c) 2017 Xiao yang <yangx.jy@cn.fujitsu.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TST_SAFE_SYSV_IPC_H__
#define TST_SAFE_SYSV_IPC_H__

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
#define SAFE_MSGCTL(msqid, cmd, buf) do { \
	safe_msgctl(__FILE__, __LINE__, (msqid), (cmd), (buf)); \
	(msqid) = ((cmd) == IPC_RMID ? -1 : (msqid)); \
	} while (0)

#endif /* TST_SAFE_SYSV_IPC_H__ */
