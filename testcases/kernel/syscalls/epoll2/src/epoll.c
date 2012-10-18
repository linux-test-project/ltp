/*
 *  epoll.c ( Efficent event polling implementation )
 *  Copyright (C) 2001,...,2002	 Davide Libenzi
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Davide Libenzi <davidel@xmailserver.org>
 *
 */

#include <linux/unistd.h>
#include <errno.h>
#include "epoll.h"

#define __sys_epoll_create(size) _syscall1(int, epoll_create, int, size)
#define __sys_epoll_ctl(epfd, op, fd, event) _syscall4(int, epoll_ctl, \
			int, epfd, int, op, int, fd, struct epoll_event *, event)
#define __sys_epoll_wait(epfd, pevents, maxevents, timeout) _syscall4(int, epoll_wait, \
			  int, epfd, struct epoll_event *, pevents, int, maxevents, int, timeout)

__sys_epoll_create(size)

    __sys_epoll_ctl(epfd, op, fd, event)

    __sys_epoll_wait(epfd, pevents, maxevents, timeout)
