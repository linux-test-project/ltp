// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2007
 * Copyright (c) 2014 Fujitsu Ltd.
 */

#ifndef RENAMEAT_H
#define RENAMEAT_H

#include <sys/types.h>
#include "config.h"
#include "lapi/syscalls.h"

#if !defined(HAVE_RENAMEAT)
int renameat(int olddirfd, const char *oldpath, int newdirfd,
			const char *newpath)
{
	return ltp_syscall(__NR_renameat, olddirfd, oldpath, newdirfd,
					newpath);
}
#endif

#endif /* RENAMEAT_H */
