// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
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

#ifndef GETDENTS_H
#define GETDENTS_H

#include <stdint.h>
#include "config.h"
#include "lapi/syscalls.h"

#if HAVE_GETDENTS || HAVE_GETDENTS64
#include <unistd.h>
#endif

/*
 * See fs/compat.c struct compat_linux_dirent
 */
struct linux_dirent {
	unsigned long   d_ino;
	unsigned long   d_off;
	unsigned short  d_reclen;
	char            d_name[];
};

static inline int
linux_getdents(unsigned int fd, struct linux_dirent *dirp, unsigned int size)
{
	return tst_syscall(__NR_getdents, fd, dirp, size);
}

struct linux_dirent64 {
	uint64_t	d_ino;
	int64_t		d_off;
	unsigned short	d_reclen;
	unsigned char	d_type;
	char		d_name[];
};

static inline int
linux_getdents64(unsigned int fd, struct linux_dirent64 *dirp64, unsigned int size)
{
	return tst_syscall(__NR_getdents64, fd, dirp64, size);
}

static inline size_t
tst_dirp_size(void)
{
	switch (tst_variant) {
	case 0:
		return sizeof(struct linux_dirent) + NAME_MAX;
	case 1:
		return sizeof(struct linux_dirent64) + NAME_MAX;
#if HAVE_GETDENTS
	case 2:
		return sizeof(struct dirent);
#endif
#if HAVE_GETDENTS64
	case 3:
		return sizeof(struct dirent64);
#endif
	}
	return 0;
}

static inline const char *
tst_dirp_name(void *dirp)
{
	switch (tst_variant) {
	case 0:
		return ((struct linux_dirent *)dirp)->d_name;
	case 1:
		return ((struct linux_dirent64 *)dirp)->d_name;
#if HAVE_GETDENTS
	case 2:
		return ((struct dirent *)dirp)->d_name;
#endif
#if HAVE_GETDENTS64
	case 3:
		return ((struct dirent64 *)dirp)->d_name;
#endif
	}
	return NULL;
}

static inline size_t
tst_dirp_reclen(void *dirp)
{
	switch (tst_variant) {
	case 0:
		return ((struct linux_dirent *)dirp)->d_reclen;
	case 1:
		return ((struct linux_dirent64 *)dirp)->d_reclen;
#if HAVE_GETDENTS
	case 2:
		return ((struct dirent *)dirp)->d_reclen;
#endif
#if HAVE_GETDENTS64
	case 3:
		return ((struct dirent64 *)dirp)->d_reclen;
#endif

	}
	return 0;
}

static inline int
tst_getdents(int fd, void *dirp, unsigned int size)
{
	switch (tst_variant) {
	case 0:
		return linux_getdents(fd, dirp, size);
	case 1:
		return linux_getdents64(fd, dirp, size);
#if HAVE_GETDENTS
	case 2:
		return getdents(fd, dirp, size);
#endif
#if HAVE_GETDENTS64
	case 3:
		return getdents64(fd, dirp, size);
#endif
	}
	return -1;
}

static inline void
getdents_info(void)
{
	switch (tst_variant) {
	case 0:
		tst_res(TINFO, "Testing the SYS_getdents syscall");
		break;
	case 1:
		tst_res(TINFO, "Testing the SYS_getdents64 syscall");
		break;
	case 2:
#if HAVE_GETDENTS
		tst_res(TINFO, "Testing libc getdents()");
#else
		tst_brk(TCONF, "libc getdents() is not implemented");
#endif
		break;
	case 3:
#if HAVE_GETDENTS64
		tst_res(TINFO, "Testing libc getdents64()");
#else
		tst_brk(TCONF, "libc getdents64() is not implemented");
#endif
		break;
	}
}

#define TEST_VARIANTS 4

#endif /* GETDENTS_H */
