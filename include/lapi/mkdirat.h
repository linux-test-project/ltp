// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef LAPI_MKDIRAT_H__
#define LAPI_MKDIRAT_H__

#include "config.h"
#include "lapi/syscalls.h"
#include "lapi/fcntl.h"

#ifndef HAVE_MKDIRAT
static inline int mkdirat(int dirfd, const char *dirname, int mode)
{
	return ltp_syscall(__NR_mkdirat, dirfd, dirname, mode);
}
#endif

#endif /* LAPI_MKDIRAT_H__ */
