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

static int do_epoll_pwait(int epfd, struct epoll_event *events, int
	maxevents, struct timespec *timeout, const sigset_t *sigmask)
{
	if (tst_variant == 1)
		return epoll_pwait2(epfd, events, maxevents, timeout, sigmask);

	int timeout_ms = -1;

	if (timeout) {
		timeout_ms = timeout->tv_sec * MSEC_PER_SEC;
		timeout_ms += (timeout->tv_nsec + NSEC_PER_MSEC - 1) /
			NSEC_PER_MSEC;
	}

	return epoll_pwait(epfd, events, maxevents, timeout_ms, sigmask);
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
