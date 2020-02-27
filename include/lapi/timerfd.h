// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2007
 * Copyright (c) 2014 Fujitsu Ltd.
 */

#ifndef TIMERFD_H
#define TIMERFD_H

#include <time.h>
#include "config.h"
#include "lapi/syscalls.h"

#ifdef HAVE_SYS_TIMERFD_H
#include <sys/timerfd.h>
#endif

#if !defined(HAVE_TIMERFD_CREATE)
int timerfd_create(int clockid, int flags)
{
	return ltp_syscall(__NR_timerfd_create, clockid, flags);
}
#endif

#if !defined(HAVE_TIMERFD_GETTIME)
int timerfd_settime(int fd, int flags, const struct itimerspec *new_value,
		    struct itimerspec *old_value)
{
	return ltp_syscall(__NR_timerfd_settime, fd, flags, new_value,
			   old_value);
}
#endif

#if !defined(HAVE_TIMERFD_SETTIME)
int timerfd_gettime(int fd, struct itimerspec *curr_value)
{
	return ltp_syscall(__NR_timerfd_gettime, fd, curr_value);
}
#endif

#endif /* TIMERFD_H */
