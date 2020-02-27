// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2007
 * Copyright (c) 2014 Fujitsu Ltd.
 */

#ifndef SPLICE_H
#define SPLICE_H

#include "config.h"
#include "lapi/syscalls.h"

#if !defined(HAVE_SPLICE)
ssize_t splice(int fd_in, loff_t *off_in, int fd_out,
	loff_t *off_out, size_t len, unsigned int flags)
{
	return tst_syscall(__NR_splice, fd_in, off_in,
		fd_out, off_out, len, flags);
}
#endif

#endif /* SPLICE_H */
