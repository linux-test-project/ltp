// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef LAPI_READLINKAT_H__
#define LAPI_READLINKAT_H__

#include "config.h"
#include "lapi/syscalls.h"
#include "lapi/fcntl.h"

#ifndef HAVE_READLINKAT
static inline int readlinkat(int dirfd, const char *pathname,
                             char *buf, size_t bufsiz)
{
	return ltp_syscall(__NR_readlinkat, dirfd, pathname, buf, bufsiz);
}
#endif

#endif /* LAPI_READLINKAT_H__ */
