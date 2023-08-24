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
	maxevents, int timeout, const sigset_t *sigmask)
{
	if (tst_variant == 0)
		return epoll_pwait(epfd, events, maxevents, timeout, sigmask);

	struct timespec ts;

	if (timeout < 0) {
		return epoll_pwait2(epfd, events, maxevents, NULL, sigmask);
	} else {
		ts.tv_sec = timeout / MSEC_PER_SEC;
		ts.tv_nsec = NSEC_PER_MSEC * (timeout % MSEC_PER_SEC);
	}

	return epoll_pwait2(epfd, events, maxevents, &ts, sigmask);

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
