// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 SUSE LLC <rpalethorpe@suse.com>
 */

#include <sys/epoll.h>

#ifndef TST_EPOLL_H
#define TST_EPOLL_H

typedef int (*tst_on_epoll_fn)(void *, uint32_t);
struct tst_epoll_event_data {
	tst_on_epoll_fn on_epoll;
	void *self;
};

int safe_epoll_create1(const char *const file, const int lineno,
		       const int flags);

#define SAFE_EPOLL_CREATE1(flags) \
	safe_epoll_create1(__FILE__, __LINE__, (flags))

int safe_epoll_ctl(const char *const file, const int lineno,
		   int epfd, int op, int fd, struct epoll_event *ev);

#define SAFE_EPOLL_CTL(epfd, op, fd, ev) \
	safe_epoll_ctl(__FILE__, __LINE__, epfd, op, fd, ev)

int safe_epoll_wait(const char *const file, const int lineno,
		    int epfd, struct epoll_event *events,
		    int maxevents, int timeout);

#define SAFE_EPOLL_WAIT(epfd, events, maxevents, timeout)\
	safe_epoll_wait(__FILE__, __LINE__, epfd, events, maxevents, timeout)

#endif
