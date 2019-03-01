// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
 * Author: Jinhui Huang <huangjh.jy@cn.fujitsu.com>
 */

#ifndef PWRITEV2_H
#define PWRITEV2_H

#include "config.h"
#include "lapi/syscalls.h"

#if !defined(HAVE_PWRITEV2)

/* LO_HI_LONG taken from glibc */
# define LO_HI_LONG(val) (long) (val), (long) (((uint64_t) (val)) >> 32)

ssize_t pwritev2(int fd, const struct iovec *iov, int iovcnt, off_t offset,
		int flags)
{
	return tst_syscall(__NR_pwritev2, fd, iov, iovcnt,
			   LO_HI_LONG(offset), flags);
}
#endif

#endif /* PWRITEV2_H */
