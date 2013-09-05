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
#include "compat_gid.h"
#include "linux_syscall_numbers.h"

/* If the platform has __NR_sys_name32 defined it
 * means that __NR_sys_name is a 16-bit version of
 * sys_name() syscall
 */
#ifdef TST_USE_COMPAT16_SYSCALL
# define LTP_CREATE_SYSCALL(sys_name, cleanup, ...) \
	if (__NR_##sys_name##32 != __LTP__NR_INVALID_SYSCALL) { \
		return ltp_syscall(__NR_##sys_name, ##__VA_ARGS__); \
	} else { \
		tst_brkm(TCONF, cleanup, \
			"16-bit version of %s() is not supported on your " \
			"platform", #sys_name); \
	}
#else
# define LTP_CREATE_SYSCALL(sys_name, cleanup, ...) \
	return sys_name(__VA_ARGS__)
#endif

int SETGROUPS(void (cleanup)(void), size_t gidsetsize, GID_T *list)
{
	LTP_CREATE_SYSCALL(setgroups, cleanup, gidsetsize, list);
}

int GETGROUPS(void (cleanup)(void), size_t gidsetsize, GID_T *list)
{
	LTP_CREATE_SYSCALL(getgroups, cleanup, gidsetsize, list);
}

#endif /* __LTP_COMPAT_16_H__ */
