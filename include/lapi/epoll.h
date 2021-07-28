/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) 2021 Xie Ziyao <xieziyao@huawei.com>
 */

#ifndef LAPI_EPOLL_H__
#define LAPI_EPOLL_H__

#include "lapi/syscalls.h"
#include "tst_timer.h"

#ifndef EPOLL_CLOEXEC
#define EPOLL_CLOEXEC 02000000
#endif

static inline void epoll_pwait_supported(void)
{
	/* allow the tests to fail early */
	tst_syscall(__NR_epoll_pwait);
}

#ifndef HAVE_EPOLL_PWAIT
static inline int epoll_pwait(int epfd, struct epoll_event *events,
			      int maxevents, int timeout,
			      const sigset_t *sigmask)
{
	return tst_syscall(__NR_epoll_pwait, epfd, events, maxevents,
			   timeout, sigmask, _NSIG / 8);
}
#endif

static inline void epoll_pwait2_supported(void)
{
	/* allow the tests to fail early */
	tst_syscall(__NR_epoll_pwait2);
}

#ifndef HAVE_EPOLL_PWAIT2
static inline int epoll_pwait2(int epfd, struct epoll_event *events,
			       int maxevents, const struct timespec *timeout,
			       const sigset_t *sigmask)
{
	if (timeout == NULL)
		return tst_syscall(__NR_epoll_pwait2, epfd, events, maxevents,
				   NULL, sigmask, _NSIG / 8);

	struct __kernel_timespec ts;

	ts.tv_sec = timeout->tv_sec;
	ts.tv_nsec = timeout->tv_nsec;

	return tst_syscall(__NR_epoll_pwait2, epfd, events, maxevents,
			   &ts, sigmask, _NSIG / 8);
}
#endif

#endif /* LAPI_EPOLL_H__ */
