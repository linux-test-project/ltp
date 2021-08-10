// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Linaro Limited. All rights reserved.
 * Author: Viresh Kumar <viresh.kumar@linaro.org>
 */

#ifndef LAPI_PIDFD_OPEN_H__
#define LAPI_PIDFD_OPEN_H__

#include <sys/syscall.h>
#include <sys/types.h>

#include "lapi/syscalls.h"

#include "config.h"

#ifndef HAVE_PIDFD_OPEN
static inline int pidfd_open(pid_t pid, unsigned int flags)
{
	return tst_syscall(__NR_pidfd_open, pid, flags);
}
#endif

#endif /* LAPI_PIDFD_OPEN_H__ */
