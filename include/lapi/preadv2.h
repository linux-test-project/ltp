// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

#ifndef PREADV2_H
#define PREADV2_H

#include "config.h"
#include "lapi/syscalls.h"

#ifndef RWF_NOWAIT
# define RWF_NOWAIT 0x00000008
#endif

#if !defined(HAVE_PREADV2)

/* LO_HI_LONG taken from glibc */
# define LO_HI_LONG(val) (long) (val), (long) (((uint64_t) (val)) >> 32)

ssize_t preadv2(int fd, const struct iovec *iov, int iovcnt, off_t offset,
		int flags)
{
	return tst_syscall(__NR_preadv2, fd, iov, iovcnt,
			   LO_HI_LONG(offset), flags);
}
#endif

#endif /* PREADV2_H */
