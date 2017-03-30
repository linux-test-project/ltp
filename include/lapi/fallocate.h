/*
 * Copyright (c) International Business Machines  Corp., 2007
 * Copyright (c) 2014 Fujitsu Ltd.
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 */

#ifndef FALLOCATE_H
#define FALLOCATE_H

#include <sys/types.h>
#include <endian.h>
#include "config.h"
#include "lapi/abisize.h"
#include "linux_syscall_numbers.h"

#ifndef SEEK_HOLE
# define SEEK_HOLE 4
#endif

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

# ifdef __TEST_H__
#  define TST_SYSCALL_WRAPPER ltp_syscall
# else
#  define TST_SYSCALL_WRAPPER tst_syscall
# endif /* __TEST_H__ */

static inline long fallocate(int fd, int mode, loff_t offset, loff_t len)
{
	/* Deal with 32bit ABIs that have 64bit syscalls. */
# if LTP_USE_64_ABI
	return TST_SYSCALL_WRAPPER(__NR_fallocate, fd, mode, offset, len);
# else
	return (long)TST_SYSCALL_WRAPPER(__NR_fallocate, fd, mode,
				 __LONG_LONG_PAIR((off_t) (offset >> 32),
						  (off_t) offset),
				 __LONG_LONG_PAIR((off_t) (len >> 32),
						  (off_t) len));
# endif
}
#endif

#endif /* FALLOCATE_H */
