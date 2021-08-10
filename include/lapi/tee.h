// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2007
 * Copyright (c) 2014 Fujitsu Ltd.
 */

#ifndef LAPI_TEE_H__
#define LAPI_TEE_H__

#include "config.h"
#include "lapi/syscalls.h"

#if !defined(HAVE_TEE)
static inline ssize_t tee(int fd_in, int fd_out,
                          size_t len, unsigned int flags)
{
	return tst_syscall(__NR_tee, fd_in, fd_out, len, flags);
}
#endif

#endif /* LAPI_TEE_H__ */
