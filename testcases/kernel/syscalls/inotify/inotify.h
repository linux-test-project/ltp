/*
 * inotify testcase common definitions.
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
 * Ngie Cooper, April 2012
 */

#ifndef	_INOTIFY_H
#define	_INOTIFY_H

/* inotify(7) wrappers */

#if __NR_inotify_init != __LTP__NR_INVALID_SYSCALL
#define	myinotify_init() \
	ltp_syscall(__NR_inotify_init)
#else
#define	myinotify_init() \
	ltp_syscall(__NR_inotify_init1, 0)
#endif

#define	myinotify_add_watch(fd, pathname, mask)	\
	ltp_syscall(__NR_inotify_add_watch, fd, pathname, mask)

#define	myinotify_rm_watch(fd, wd) \
	ltp_syscall(__NR_inotify_rm_watch, fd, wd)

#endif /* _INOTIFY_H */
