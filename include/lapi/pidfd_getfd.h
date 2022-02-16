// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

#ifndef LAPI_PIDFD_GETFD_H__
#define LAPI_PIDFD_GETFD_H__

#include "config.h"
#include "lapi/syscalls.h"

static inline void pidfd_getfd_supported(void)
{
	/* allow the tests to fail early */
	tst_syscall(__NR_pidfd_getfd);
}

#ifndef HAVE_PIDFD_GETFD
static inline int pidfd_getfd(int pidfd, int targetfd, unsigned int flags)
{
	return tst_syscall(__NR_pidfd_getfd, pidfd, targetfd, flags);
}
#endif

#endif /* LAPI_PIDFD_GETFD_H__ */
