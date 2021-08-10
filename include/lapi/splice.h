// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2007
 * Copyright (c) 2014 Fujitsu Ltd.
 */

#ifndef LAPI_SPLICE_H__
#define LAPI_SPLICE_H__

#include "config.h"
#include "lapi/syscalls.h"

#if !defined(HAVE_SPLICE)
static inline ssize_t splice(int fd_in, loff_t *off_in, int fd_out,
                             loff_t *off_out, size_t len, unsigned int flags)
{
	return tst_syscall(__NR_splice, fd_in, off_in,
		fd_out, off_out, len, flags);
}
#endif

#endif /* LAPI_SPLICE_H__ */
