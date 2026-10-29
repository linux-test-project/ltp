// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

#ifndef LAPI_PREADV2_H__
#define LAPI_PREADV2_H__

#include <sys/uio.h>
#include "config.h"
#include "lapi/syscalls.h"

#ifndef RWF_NOWAIT
# define RWF_NOWAIT 0x00000008
#endif


/* LO_HI_LONG taken from glibc */
# define LO_HI_LONG(val) (long) (val), (long) (((uint64_t) (val)) >> 32)

#if !defined(HAVE_PREADV)
static inline ssize_t preadv(int fd, const struct iovec *iov, int iovcnt,
	off_t offset)
{
	return tst_syscall(__NR_preadv, fd, iov, iovcnt, LO_HI_LONG(offset));
}
#endif

#if !defined(HAVE_PWRITEV)
static inline ssize_t pwritev(int fd, const struct iovec *iov, int iovcnt,
	off_t offset)
{
	return tst_syscall(__NR_pwritev, fd, iov, iovcnt, LO_HI_LONG(offset));
}
#endif

#if !defined(HAVE_PREADV2)
static inline ssize_t preadv2(int fd, const struct iovec *iov, int iovcnt,
                              off_t offset, int flags)
{
	return tst_syscall(__NR_preadv2, fd, iov, iovcnt,
			   LO_HI_LONG(offset), flags);
}
#endif

#if !defined(HAVE_PWRITEV2)
static inline ssize_t pwritev2(int fd, const struct iovec *iov, int iovcnt,
                               off_t offset, int flags)
{
	return tst_syscall(__NR_pwritev2, fd, iov, iovcnt,
			   LO_HI_LONG(offset), flags);
}
#endif

#undef LO_HI_LONG

#endif /* LAPI_PREADV2_H__ */
