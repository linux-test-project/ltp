/*
 * Copyright (c) International Business Machines  Corp., 2007
 * Copyright (c) 2014 Fujitsu Ltd.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
