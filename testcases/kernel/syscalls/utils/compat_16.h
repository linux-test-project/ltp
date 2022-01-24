/*
 * Copyright (c) Red Hat Inc., 2008
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* Author: Masatake YAMATO <yamato@redhat.com> */

#ifndef __LTP_COMPAT_16_H__
#define __LTP_COMPAT_16_H__

#include <errno.h>
#include <grp.h>
#include <sys/fsuid.h>
#include <sys/types.h>
#include <unistd.h>

#include "compat_gid.h"
#include "compat_uid.h"
#include "lapi/syscalls.h"

int setresuid(uid_t ruid, uid_t euid, uid_t suid);
int getresuid(uid_t *ruid, uid_t *euid, uid_t *suid);
int setresgid(gid_t rgid, gid_t egid, gid_t sgid);
int getresgid(gid_t *rgid, gid_t *egid, gid_t *sgid);


/* If the platform has __NR_sys_name32 defined it
 * means that __NR_sys_name is a 16-bit version of
 * sys_name() syscall
 */
#ifdef TST_USE_COMPAT16_SYSCALL
# define LTP_CREATE_SYSCALL(sys_name, cleanup, ...) \
	if (__NR_##sys_name##32 != __LTP__NR_INVALID_SYSCALL) { \
		return tst_syscall(__NR_##sys_name, ##__VA_ARGS__); \
	} else { \
		tst_brkm(TCONF, cleanup, \
			"16-bit version of %s() is not supported on your " \
			"platform", #sys_name); \
	}
#else
# define LTP_CREATE_SYSCALL(sys_name, cleanup, ...) \
	(void) cleanup;                             \
	return sys_name(__VA_ARGS__)
#endif

#define UID16_CHECK(uid, sys_name, cleanup) \
if (!UID_SIZE_CHECK(uid)) { \
	tst_brkm(TBROK, cleanup, \
		"uid %d of %s is too large for testing 16-bit version " \
		"of %s()", uid, #uid, #sys_name); \
}

#define GID16_CHECK(gid, sys_name, cleanup) \
if (!GID_SIZE_CHECK(gid)) { \
	tst_brkm(TBROK, cleanup, \
		"gid %d of %s is too large for testing 16-bit version " \
		"of %s()", gid, #gid, #sys_name); \
}


int SETGROUPS(void (cleanup)(void), size_t gidsetsize, GID_T *list)
{
	LTP_CREATE_SYSCALL(setgroups, cleanup, gidsetsize, list);
}

int GETGROUPS(void (cleanup)(void), size_t gidsetsize, GID_T *list)
{
	LTP_CREATE_SYSCALL(getgroups, cleanup, gidsetsize, list);
}

int SETUID(void (cleanup)(void), UID_T uid)
{
	LTP_CREATE_SYSCALL(setuid, cleanup, uid);
}

UID_T GETUID(void (cleanup)(void))
{
	LTP_CREATE_SYSCALL(getuid, cleanup);
}

int SETGID(void (cleanup)(void), GID_T gid)
{
	LTP_CREATE_SYSCALL(setgid, cleanup, gid);
}

GID_T GETGID(void (cleanup)(void))
{
	LTP_CREATE_SYSCALL(getgid, cleanup);
}

UID_T GETEUID(void (cleanup)(void))
{
	LTP_CREATE_SYSCALL(geteuid, cleanup);
}

GID_T GETEGID(void (cleanup)(void))
{
	LTP_CREATE_SYSCALL(getegid, cleanup);
}

int SETFSUID(void (cleanup)(void), UID_T uid)
{
	LTP_CREATE_SYSCALL(setfsuid, cleanup, uid);
}

int SETFSGID(void (cleanup)(void), GID_T gid)
{
	LTP_CREATE_SYSCALL(setfsgid, cleanup, gid);
}

int SETREUID(void (cleanup)(void), UID_T ruid, UID_T euid)
{
	LTP_CREATE_SYSCALL(setreuid, cleanup, ruid, euid);
}
int SETREGID(void (cleanup)(void), GID_T rgid, GID_T egid)
{
	LTP_CREATE_SYSCALL(setregid, cleanup, rgid, egid);
}

int SETRESUID(void (cleanup)(void), UID_T ruid, UID_T euid, UID_T suid)
{
	LTP_CREATE_SYSCALL(setresuid, cleanup, ruid, euid, suid);
}

int GETRESUID(void (cleanup)(void), UID_T *ruid, UID_T *euid, UID_T *suid)
{
	LTP_CREATE_SYSCALL(getresuid, cleanup, ruid, euid, suid);
}

int SETRESGID(void (cleanup)(void), GID_T rgid, GID_T egid, GID_T sgid)
{
	LTP_CREATE_SYSCALL(setresgid, cleanup, rgid, egid, sgid);
}

int GETRESGID(void (cleanup)(void), GID_T *rgid, GID_T *egid, GID_T *sgid)
{
	LTP_CREATE_SYSCALL(getresgid, cleanup, rgid, egid, sgid);
}

int FCHOWN(void (cleanup)(void), unsigned int fd, UID_T owner, GID_T group)
{
	LTP_CREATE_SYSCALL(fchown, cleanup, fd, owner, group);
}

int LCHOWN(void (cleanup)(void), const char *path, UID_T owner, GID_T group)
{
	LTP_CREATE_SYSCALL(lchown, cleanup, path, owner, group);
}

int CHOWN(void (cleanup)(void), const char *path, UID_T owner, GID_T group)
{
	LTP_CREATE_SYSCALL(chown, cleanup, path, owner, group);
}
#endif /* __LTP_COMPAT_16_H__ */
