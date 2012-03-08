/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
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

/*
 * getdents.h - common definitions for the getdents() tests.
 */

#ifndef __GETDENTS_H
#define __GETDENTS_H	1

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>

/*
 * The dirent struct that the C library exports is not the same
 * as the kernel ABI, so we can't include dirent.h and use the
 * dirent struct from there.  Further, since the Linux headers
 * don't export their vision of the struct either, we have to
 * declare our own here.  Wheeeeee.
 */

struct linux_dirent {
	unsigned long   d_ino;
	unsigned long   d_off;
	unsigned short  d_reclen;
	char            d_name[];
};

static inline int
getdents(unsigned int fd, struct dirent *dirp, unsigned int count)
{
	union {
		struct linux_dirent *dirp;
		char *buf;
	} ptrs;
	char buf[count];
	long ret;
	unsigned int i;

	ptrs.buf = buf;
	ret = syscall(SYS_getdents, fd, buf, count);
	if (ret < 0)
		return ret;

#define kdircpy(field) memcpy(&dirp[i].field, &ptrs.dirp->field, sizeof(dirp[i].field))

	i = 0;
	while (i < count && i < ret) {
		unsigned long reclen;

		kdircpy(d_ino);
		kdircpy(d_reclen);
		reclen = dirp[i].d_reclen;
		kdircpy(d_off);
		strcpy(dirp[i].d_name, ptrs.dirp->d_name);

		ptrs.buf += reclen;

		i += reclen;
	}

	return ret;
}

#endif /* getdents.h */
