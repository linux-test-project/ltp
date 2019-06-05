// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * inotify testcase common definitions.
 *
 * Copyright (c) 2012-2019 Linux Test Project.  All Rights Reserved.
 * Ngie Cooper, April 2012
 */

#ifndef	_INOTIFY_H
#define	_INOTIFY_H

#include "lapi/syscalls.h"

/* inotify(7) wrappers */

#if __NR_inotify_init != __LTP__NR_INVALID_SYSCALL
#define myinotify_init() \
	tst_syscall(__NR_inotify_init)
#else
#define myinotify_init() \
	tst_syscall(__NR_inotify_init1, 0)
#endif

#define myinotify_init1(flags) \
	tst_syscall(__NR_inotify_init1, flags)

#define myinotify_add_watch(fd, pathname, mask)	\
	tst_syscall(__NR_inotify_add_watch, fd, pathname, mask)

#define myinotify_rm_watch(fd, wd) \
	tst_syscall(__NR_inotify_rm_watch, fd, wd)

static inline int safe_myinotify_init(const char *file, const int lineno, int fd)
{
	if (fd < 0) {
		if (errno == ENOSYS) {
			tst_brk(TCONF,
				"%s:%d: inotify is not configured in this kernel",
				file, lineno);
		} else {
			tst_brk(TBROK | TERRNO, "%s:%d: inotify_init failed",
				file, lineno);
		}
	}
	return fd;
}

#define SAFE_MYINOTIFY_INIT() \
	safe_myinotify_init(__FILE__, __LINE__, myinotify_init())

#define SAFE_MYINOTIFY_INIT1(flags) \
	safe_myinotify_init(__FILE__, __LINE__, myinotify_init1(flags))

static inline int safe_myinotify_watch(const char *file, const int lineno, int wd, int fd, const char* fname, const char* mask)
{
	if (wd < 0) {
		tst_brk(TBROK | TERRNO,
			"%s:%d: inotify_add_watch (%d, %s, %s) failed",
			file, lineno, fd, fname, mask);
	}
	return wd;
}

#define SAFE_MYINOTIFY_ADD_WATCH(fd, pathname, mask)	\
	safe_myinotify_watch(__FILE__, __LINE__, myinotify_add_watch(fd, pathname, mask), fd, pathname, #mask)

#endif /* _INOTIFY_H */
