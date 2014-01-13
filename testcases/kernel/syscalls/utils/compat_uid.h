/*
 *
 *   Copyright (c) Red Hat Inc., 2009
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

#ifndef __COMPAT_UID_16_H__
#define __COMPAT_UID_16_H__


#include <asm/posix_types.h>


#ifdef TST_USE_COMPAT16_SYSCALL
typedef __kernel_old_uid_t UID_T;
int
UID_SIZE_CHECK(uid_t uid)
{
	/* See high2lowuid in linux/highuid.h
	   Return 0 if uid is too large to store
	   it to __kernel_old_uid_t. */
	return ((uid) & ~0xFFFF)? 0: 1;
}

#else

typedef uid_t UID_T;
int
UID_SIZE_CHECK(uid_t uid)
{
	return 1;
}

#endif

/* for 16-bit syscalls testing we can only
 * use uids <= 0xFFFE */
uid_t GET_UNUSED_UID(void)
{
	uid_t r;
	r = tst_get_unused_uid();

#ifdef TST_USE_COMPAT16_SYSCALL
	if (!UID_SIZE_CHECK(r))
		return -1;

	/* kernel low2highuid() converts
	 * 0xFFFF to (uid_t)-1 */
	if (r == (UID_T)-1)
		return -1;
#endif

	return r;
}

#endif /* __SETUID_COMPAT_16_H__ */
