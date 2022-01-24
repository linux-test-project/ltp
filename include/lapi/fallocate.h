// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2007
 * Copyright (c) 2014 Fujitsu Ltd.
 */

#ifndef LAPI_FALLOCATE_H__
#define LAPI_FALLOCATE_H__

#include <sys/types.h>
#include <endian.h>
#include "config.h"
#include "lapi/abisize.h"
#include "lapi/seek.h"
#include "lapi/syscalls.h"

#ifndef FALLOC_FL_KEEP_SIZE
# define FALLOC_FL_KEEP_SIZE 0x01
#endif

#ifndef FALLOC_FL_PUNCH_HOLE
# define FALLOC_FL_PUNCH_HOLE 0x02
#endif

#ifndef FALLOC_FL_COLLAPSE_RANGE
# define FALLOC_FL_COLLAPSE_RANGE 0x08
#endif

#ifndef FALLOC_FL_ZERO_RANGE
# define FALLOC_FL_ZERO_RANGE 0x10
#endif

#ifndef FALLOC_FL_INSERT_RANGE
# define FALLOC_FL_INSERT_RANGE 0x20
#endif

#if !defined(HAVE_FALLOCATE)

static inline long fallocate(int fd, int mode, loff_t offset, loff_t len)
{
	/* Deal with 32bit ABIs that have 64bit syscalls. */
# if LTP_USE_64_ABI
	return tst_syscall(__NR_fallocate, fd, mode, offset, len);
# else
	return (long)tst_syscall(__NR_fallocate, fd, mode,
				 __LONG_LONG_PAIR((off_t) (offset >> 32),
						  (off_t) offset),
				 __LONG_LONG_PAIR((off_t) (len >> 32),
						  (off_t) len));
# endif
}
#endif

#endif /* LAPI_FALLOCATE_H__ */
