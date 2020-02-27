// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2018 MediaTek Inc.  All Rights Reserved.
 */

#ifndef EXECVEAT_H
#define EXECVEAT_H

#include <sys/types.h>
#include "config.h"
#include "lapi/syscalls.h"

#if !defined(HAVE_EXECVEAT)
int execveat(int dirfd, const char *pathname,
			char *const argv[], char *const envp[],
			int flags)
{
	return tst_syscall(__NR_execveat, dirfd, pathname, argv, envp, flags);
}
#endif

#endif /* EXECVEAT_H */
