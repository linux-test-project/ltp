/*
 * fanotify testcase common definitions.
 *
 * Copyright (c) 2012 Linux Test Project.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Jan Kara, November 2013
 */

#ifndef	__FANOTIFY_H__
#define	__FANOTIFY_H__

#include "config.h"

#if defined(HAVE_SYS_FANOTIFY_H)

#include <sys/fanotify.h>

#else /* HAVE_SYS_FANOTIFY_H */

/* fanotify(7) wrappers */

#include <stdint.h>
#include "linux_syscall_numbers.h"

static int fanotify_init(unsigned int flags, unsigned int event_f_flags)
{
	return syscall(__NR_fanotify_init, flags, event_f_flags);
}

static long fanotify_mark(int fd, unsigned int flags, uint64_t mask,
                     int dfd, const char *pathname)
{
	return syscall(__NR_fanotify_mark, fd, flags, mask, dfd, pathname);
}

#endif /* HAVE_SYS_FANOTIFY_H */

#endif /* __FANOTIFY_H__ */
