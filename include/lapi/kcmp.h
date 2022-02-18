// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015 Cedric Hnyda <chnyda@suse.com>
 */

#ifndef LAPI_KCMP_H__
#define LAPI_KCMP_H__

#include <sys/types.h>
#include "config.h"
#include "lapi/syscalls.h"

#ifndef HAVE_ENUM_KCMP_TYPE

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

#ifndef HAVE_KCMP

static inline int kcmp(int pid1, int pid2, int type, int fd1, int fd2)
{
	return tst_syscall(__NR_kcmp, pid1, pid2, type, fd1, fd2);
}

#endif

#endif /* LAPI_KCMP_H__ */
