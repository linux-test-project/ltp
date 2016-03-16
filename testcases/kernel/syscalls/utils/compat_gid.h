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

#ifndef __COMPAT_GID_16_H__
#define __COMPAT_GID_16_H__

#include <asm/posix_types.h>
#include "tst_common.h"

#ifdef TST_USE_COMPAT16_SYSCALL
typedef __kernel_old_gid_t GID_T;
int GID_SIZE_CHECK(gid_t gid)
{
	/* See high2lowgid in linux/highuid.h
	   Return 0 if gid is too large to store
	   it to __kernel_old_gid_t. */
	return ((gid) & ~0xFFFF)? 0: 1;
}

#else

typedef gid_t GID_T;
int GID_SIZE_CHECK(gid_t gid LTP_ATTRIBUTE_UNUSED)
{
	return 1;
}

#endif

#endif /* __SETGID_COMPAT_16_H__ */
