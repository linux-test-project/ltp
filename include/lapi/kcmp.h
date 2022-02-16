/*
 * Copyright (c) 2015 Cedric Hnyda <chnyda@suse.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef KCMP_H
#define KCMP_H

#include <sys/types.h>
#include "config.h"
#include "lapi/syscalls.h"

#if !defined(HAVE_ENUM_KCMP_TYPE)

enum kcmp_type {
	KCMP_FILE,
	KCMP_VM,
	KCMP_FILES,
	KCMP_FS,
	KCMP_SIGHAND,
	KCMP_IO,
	KCMP_SYSVSEM,
	KCMP_TYPES,
};

#else

# include <linux/kcmp.h>

#endif

#if !defined(HAVE_KCMP)

int kcmp(int pid1, int pid2, int type, int fd1, int fd2)
{
	return tst_syscall(__NR_kcmp, pid1, pid2, type, fd1, fd2);
}

#endif

#endif /* KCMP_H */
