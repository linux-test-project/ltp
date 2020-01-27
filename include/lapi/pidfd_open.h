// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Linaro Limited. All rights reserved.
 * Author: Viresh Kumar <viresh.kumar@linaro.org>
 */

#ifndef PIDFD_OPEN_H
#define PIDFD_OPEN_H

#include <sys/syscall.h>
#include <sys/types.h>

#include "lapi/syscalls.h"

#include "config.h"

#ifndef HAVE_PIDFD_OPEN
int pidfd_open(pid_t pid, unsigned int flags)
{
	return tst_syscall(__NR_pidfd_open, pid, flags);
}
#endif

#endif /* PIDFD_OPEN_H */
