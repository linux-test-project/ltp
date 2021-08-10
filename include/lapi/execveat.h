// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2018 MediaTek Inc.  All Rights Reserved.
 */

#ifndef LAPI_EXECVEAT_H__
#define LAPI_EXECVEAT_H__

#include <sys/types.h>
#include "config.h"
#include "lapi/syscalls.h"

#if !defined(HAVE_EXECVEAT)
static inline int execveat(int dirfd, const char *pathname,
			   char *const argv[], char *const envp[],
			   int flags)
{
	return tst_syscall(__NR_execveat, dirfd, pathname, argv, envp, flags);
}
#endif

#endif /* LAPI_EXECVEAT_H__ */
