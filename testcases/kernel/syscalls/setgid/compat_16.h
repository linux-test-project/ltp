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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* Author: Masatake YAMATO <yamato@redhat.com> */

#ifndef __SETGID_COMPAT_16_H__
#define __SETGID_COMPAT_16_H__


#include "linux_syscall_numbers.h"
#include "compat_gid.h"


#ifdef TST_USE_COMPAT16_SYSCALL

int
SETGID(GID_T gid)
{
	return ltp_syscall(__NR_setgid, gid);
}

GID_T
GETGID(void)
{
	gid_t gid;

	gid = getgid();
	if (!GID_SIZE_CHECK(gid))
	  tst_brkm(TBROK,
		   cleanup,
		   "gid for the current process is too large for testing setgid16");

	return (GID_T)gid;
}

#else

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
