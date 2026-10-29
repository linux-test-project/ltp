// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2007
 * Copyright (c) 2014 Fujitsu Ltd.
 */

#ifndef LAPI_SPLICE_H__
#define LAPI_SPLICE_H__

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <fcntl.h>
#include <sys/types.h>

#include "config.h"
#include "lapi/syscalls.h"

#ifndef SPLICE_F_MORE
# define SPLICE_F_MORE 4
#endif

#if !defined(HAVE_SPLICE)
static inline ssize_t splice(int fd_in, loff_t *off_in, int fd_out,
                             loff_t *off_out, size_t len, unsigned int flags)
{
	return tst_syscall(__NR_splice, fd_in, off_in,
		fd_out, off_out, len, flags);
}
#endif

static inline ssize_t safe_splice(const char *file, const int lineno,
				   int fd_in, loff_t *off_in,
				   int fd_out, loff_t *off_out,
				   size_t len, unsigned int flags)
{
	ssize_t ret;

	ret = splice(fd_in, off_in, fd_out, off_out, len, flags);

	if (ret < 0) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"splice(%d, %p, %d, %p, %zu, %u) failed",
			fd_in, off_in, fd_out, off_out, len, flags);
	}

	return ret;
}

#define SAFE_SPLICE(fd_in, off_in, fd_out, off_out, len, flags) \
	safe_splice(__FILE__, __LINE__, (fd_in), (off_in), (fd_out), \
		    (off_out), (len), (flags))

#endif /* LAPI_SPLICE_H__ */
