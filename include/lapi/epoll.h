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

#ifndef EPOLLEXCLUSIVE
# define EPOLLEXCLUSIVE (1U << 28)
#endif

static inline void epoll_pwait_supported(void)
{
	/* allow the tests to fail early */
	tst_syscall(__NR_epoll_pwait);
}

/*
 * Note: The kernel expects an _NSIG / 8 bytes signal set, which may be larger
 * than sizeof(sigset_t) on some 32-bit libcs (e.g. 4 bytes on 32-bit Bionic).
 */
static inline int sys_epoll_pwait(int epfd, struct epoll_event *events,
				  int maxevents, int timeout,
				  const sigset_t *sigmask)
{
	return tst_syscall(__NR_epoll_pwait, epfd, events, maxevents,
			   timeout, sigmask, _NSIG / 8);
}

#ifndef HAVE_EPOLL_PWAIT
static inline int epoll_pwait(int epfd, struct epoll_event *events,
			      int maxevents, int timeout,
			      const sigset_t *sigmask)
{
	return sys_epoll_pwait(epfd, events, maxevents, timeout, sigmask);
}
#endif

static inline void epoll_pwait2_supported(void)
{
	/* allow the tests to fail early */
	tst_syscall(__NR_epoll_pwait2);
}

static inline int sys_epoll_pwait2(int epfd, struct epoll_event *events,
				   int maxevents,
				   const struct timespec *timeout,
				   const sigset_t *sigmask)
{
	struct __kernel_timespec ts;

	if (!timeout) {
		return tst_syscall(__NR_epoll_pwait2, epfd, events, maxevents,
				   NULL, sigmask, _NSIG / 8);
	}

	ts.tv_sec = timeout->tv_sec;
	ts.tv_nsec = timeout->tv_nsec;

	return tst_syscall(__NR_epoll_pwait2, epfd, events, maxevents,
			   &ts, sigmask, _NSIG / 8);
}

#ifndef HAVE_EPOLL_PWAIT2
static inline int epoll_pwait2(int epfd, struct epoll_event *events,
			       int maxevents, const struct timespec *timeout,
			       const sigset_t *sigmask)
{
	return sys_epoll_pwait2(epfd, events, maxevents, timeout, sigmask);
}
#endif

#endif /* LAPI_EPOLL_H__ */
