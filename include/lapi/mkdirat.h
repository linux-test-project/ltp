// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef __MKDIRAT_H__
#define __MKDIRAT_H__

#include "config.h"
#include "lapi/syscalls.h"
#include "lapi/fcntl.h"

#ifndef HAVE_MKDIRAT
int mkdirat(int dirfd, const char *dirname, int mode)
{
	return ltp_syscall(__NR_mkdirat, dirfd, dirname, mode);
}
#endif

#endif /* __MKDIRAT_H__ */
