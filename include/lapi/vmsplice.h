// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2007
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef LAPI_VMSPLICE_H__
#define LAPI_VMSPLICE_H__

#include "config.h"
#include "lapi/syscalls.h"

#include "lapi/iovec.h"

#if !defined(HAVE_VMSPLICE)
static inline ssize_t vmsplice(int fd, const struct iovec *iov,
	                       unsigned long nr_segs, unsigned int flags)
{
	return tst_syscall(__NR_vmsplice, fd, iov, nr_segs, flags);
}
#endif

#endif /* LAPI_VMSPLICE_H__ */
