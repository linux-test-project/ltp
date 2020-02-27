// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2007
 * Copyright (c) 2014 Fujitsu Ltd.
 */

#ifndef TEE_H
#define TEE_H

#include "config.h"
#include "lapi/syscalls.h"

#if !defined(HAVE_TEE)
ssize_t tee(int fd_in, int fd_out, size_t len, unsigned int flags)
{
	return tst_syscall(__NR_tee, fd_in, fd_out, len, flags);
}
#endif

#endif /* TEE_H */
