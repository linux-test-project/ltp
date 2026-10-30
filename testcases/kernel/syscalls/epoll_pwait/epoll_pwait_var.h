/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * Author: Xie Ziyao <xieziyao@huawei.com>
 */

#ifndef LTP_EPOLL_PWAIT_VAR_H
#define LTP_EPOLL_PWAIT_VAR_H

#include "lapi/epoll.h"

#define TEST_VARIANTS 2
#define MSEC_PER_SEC (1000L)
#define NSEC_PER_MSEC (1000000L)

static inline int epoll_timeout_ms(const struct timespec *timeout)
{
	if (!timeout)
		return -1;

	return timeout->tv_sec * MSEC_PER_SEC +
		(timeout->tv_nsec + NSEC_PER_MSEC - 1) / NSEC_PER_MSEC;
}

static inline int do_epoll_pwait(int epfd, struct epoll_event *events,
	int maxevents, struct timespec *timeout, const sigset_t *sigmask)
{
	if (tst_variant == 1)
		return epoll_pwait2(epfd, events, maxevents, timeout, sigmask);

	return epoll_pwait(epfd, events, maxevents, epoll_timeout_ms(timeout),
			   sigmask);
}

static inline int do_sys_epoll_pwait(int epfd, struct epoll_event *events,
	int maxevents, struct timespec *timeout, const sigset_t *sigmask)
{
	if (tst_variant == 1) {
		return sys_epoll_pwait2(epfd, events, maxevents, timeout,
					sigmask);
	}

	return sys_epoll_pwait(epfd, events, maxevents,
			       epoll_timeout_ms(timeout), sigmask);
}

static void epoll_pwait_init(void)
{
	if (tst_variant == 0) {
		tst_res(TINFO, "Test epoll_pwait()");
		epoll_pwait_supported();
	} else {
		tst_res(TINFO, "Test epoll_pwait2()");
		epoll_pwait2_supported();
	}
}

#endif /* LTP_EPOLL_PWAIT_VAR_H */
