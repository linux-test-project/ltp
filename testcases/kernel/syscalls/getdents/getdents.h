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
#include "test.h"
#include "lapi/syscalls.h"
#include "config.h"
/*
 * See fs/compat.c struct compat_linux_dirent
 */
struct linux_dirent {
	unsigned long   d_ino;
	unsigned long   d_off;
	unsigned short  d_reclen;
	char            d_name[];
};

#if HAVE_GETDENTS
#include <unistd.h>
#else
static inline int
getdents(unsigned int fd, struct linux_dirent *dirp, unsigned int size)
{
	return ltp_syscall(__NR_getdents, fd, dirp, size);
}

#endif /* HAVE_GETDENTS */

struct linux_dirent64 {
	uint64_t	d_ino;
	int64_t		d_off;
	unsigned short	d_reclen;
	unsigned char	d_type;
	char		d_name[];
};

#if HAVE_GETDENTS64
#include <dirent.h>
#include <unistd.h>
#else
static inline int
getdents64(unsigned int fd, struct linux_dirent64 *dirp64, unsigned int size)
{
	return ltp_syscall(__NR_getdents64, fd, dirp64, size);
}
#endif /* HAVE_GETDENTS64 */
#endif /* GETDENTS_H */
