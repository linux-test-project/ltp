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

static inline void close_range_supported_by_kernel(void)
{
	long ret;

	if ((tst_kvercmp(5, 9, 0)) < 0) {
		/* Check if the syscall is backported on an older kernel */
		ret = syscall(__NR_close_range, 1, 0, 0);
		if (ret == -1 && errno == ENOSYS)
			tst_brk(TCONF, "Test not supported on kernel version < v5.9");
	}
}

#endif	/* LAPI_CLOSE_RANGE_H__ */
