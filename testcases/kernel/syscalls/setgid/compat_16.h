/*
 *
 *   Copyright (c) Red Hat Inc., 2008
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* Author: Masatake YAMATO <yamato@redhat.com> */

#ifndef __SETGID_COMPAT_16_H__
#define __SETGID_COMPAT_16_H__


#include <asm/posix_types.h>
#include "linux_syscall_numbers.h"


#ifdef TST_USE_COMPAT16_SYSCALL
typedef __kernel_old_gid_t GID_T;
int 
COMPAT_SIZE_CHECK(gid_t gid)
{
	/* See high2lowgid in linux/highuid.h
	   Return 0 if gid is too large to store
	   it to __kernel_old_gid_t. */
	return ((gid) & ~0xFFFF)? 0: 1;
}
int 
SETGID(GID_T gid)
{
	return syscall(__NR_setgid, gid);
}
GID_T
GETGID(void)
{
	gid_t gid;

	gid = getgid();
	if (!COMPAT_SIZE_CHECK(gid))
	  tst_brkm(TBROK, 
		   cleanup, 
		   "gid for the current process is too large for testing setgid16");

	return (GID_T)gid;
}
#else
typedef gid_t GID_T;
int 
COMPAT_SIZE_CHECK(gid_t gid)
{
	return 1;
}
int 
SETGID(GID_T gid)
{
	return setgid(gid);
}
GID_T
GETGID(void)
{
	return getgid();
}
#endif

#endif /* __SETGID_COMPAT_16_H__ */
