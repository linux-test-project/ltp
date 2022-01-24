// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2007
 * Copyright (c) 2014 Fujitsu Ltd.
 */

#ifndef LAPI_RENAMEAT_H__
#define LAPI_RENAMEAT_H__

#include <sys/types.h>
#include "config.h"
#include "lapi/syscalls.h"

#if !defined(HAVE_RENAMEAT)
static inline int renameat(int olddirfd, const char *oldpath, int newdirfd,
                           const char *newpath)
{
	return tst_syscall(__NR_renameat, olddirfd, oldpath, newdirfd,
					newpath);
}
#endif

#endif /* LAPI_RENAMEAT_H__ */
