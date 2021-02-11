// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) 2021 SUSE LLC  */

#ifndef LAPI_CLOSE_RANGE__
# define LAPI_CLOSE_RANGE__

# include "lapi/syscalls.h"

# ifdef HAVE_LINUX_CLOSE_RANGE_H
#  include <linux/close_range.h>
# endif

# ifndef CLOSE_RANGE_UNSHARE
#  define CLOSE_RANGE_UNSHARE	(1U << 1)
# endif

# ifndef CLOSE_RANGE_CLOEXEC
#  define CLOSE_RANGE_CLOEXEC	(1U << 2)
# endif

# ifndef HAVE_CLOSE_RANGE
static inline int close_range(unsigned int fd, unsigned int max_fd,
			      unsigned int flags)
{
	return tst_syscall(__NR_close_range, fd, max_fd, flags);
}
# endif
#endif	/* LAPI_CLOSE_RANGE_H__ */
