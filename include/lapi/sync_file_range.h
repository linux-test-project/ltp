// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2008
 */

#ifndef SYNC_FILE_RANGE_H
#define SYNC_FILE_RANGE_H

#include <sys/types.h>
#include "config.h"
#include "lapi/syscalls.h"
#include "lapi/abisize.h"

#if !defined(HAVE_SYNC_FILE_RANGE)

#ifdef TST_TEST_H__
# define TST_SYSCALL tst_syscall
#else
# define TST_SYSCALL ltp_syscall
#endif

/*****************************************************************************
 * Wraper function to call sync_file_range system call
 ******************************************************************************/
static inline long sync_file_range(int fd, off64_t offset, off64_t nbytes,
				   unsigned int flags)
{
#if (defined(__arm__) || defined(__powerpc__) || defined(__powerpc64__))
# ifdef TST_ABI32
#  if __BYTE_ORDER == __BIG_ENDIAN
	return TST_SYSCALL(__NR_sync_file_range2, fd, flags,
		(int)(offset >> 32), (int)offset, (int)(nbytes >> 32),
		(int)nbytes);
#  elif __BYTE_ORDER == __LITTLE_ENDIAN
	return TST_SYSCALL(__NR_sync_file_range2, fd, flags, (int)offset,
		       (int)(offset >> 32), nbytes, (int)(nbytes >> 32));
#  endif
# else
	return TST_SYSCALL(__NR_sync_file_range2, fd, flags, offset, nbytes);
# endif
#elif (defined(__s390__) || defined(__s390x__)) && defined(TST_ABI32)
	return TST_SYSCALL(__NR_sync_file_range, fd, (int)(offset >> 32),
		(int)offset, (int)(nbytes >> 32), (int)nbytes, flags);
#elif defined(__mips__) && defined(TST_ABI32)
# if __BYTE_ORDER == __BIG_ENDIAN
	return TST_SYSCALL(__NR_sync_file_range, fd, 0, (int)(offset >> 32),
		(int)offset, (int)(nbytes >> 32), (int)nbytes, flags);
# elif __BYTE_ORDER == __LITTLE_ENDIAN
	return TST_SYSCALL(__NR_sync_file_range, fd, 0, (int)offset,
		(int)(offset >> 32), (int)nbytes, (int)(nbytes >> 32), flags);
# endif
#else
	return TST_SYSCALL(__NR_sync_file_range, fd, offset, nbytes, flags);
#endif
}
#endif

#endif /* SYNC_FILE_RANGE_H */
