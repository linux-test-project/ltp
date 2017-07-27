/*
 * Copyright (c) 2016 Fujitsu Ltd.
 * Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.
 */

#ifndef EPOLL_PWAIT_H
#define EPOLL_PWAIT_H

#include <sys/types.h>
#include <signal.h>
#include "config.h"
#include "lapi/syscalls.h"

#if !defined(HAVE_EPOLL_PWAIT)
int epoll_pwait(int epfd, struct epoll_event *events, int maxevents,
	int timeout, const sigset_t *sigmask)
{
	return ltp_syscall(__NR_epoll_pwait, epfd, events, maxevents,
		timeout, sigmask, _NSIG / 8);
}
#endif

#endif /* EPOLL_PWAIT_H */
