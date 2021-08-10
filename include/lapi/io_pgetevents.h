// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Linaro Limited. All rights reserved.
 * Author: Viresh Kumar <viresh.kumar@linaro.org>
 */

#ifndef LAPI_IO_PGETEVENTS_H__
#define LAPI_IO_PGETEVENTS_H__

#include <sys/syscall.h>
#include <sys/types.h>

#include "config.h"
#include "lapi/syscalls.h"

#ifdef HAVE_LIBAIO
#include <libaio.h>

static inline int sys_io_pgetevents(io_context_t ctx, long min_nr, long max_nr,
		struct io_event *events, void *timeout, sigset_t *sigmask)
{
	return tst_syscall(__NR_io_pgetevents, ctx, min_nr, max_nr, events,
			   timeout, sigmask);
}

static inline int sys_io_pgetevents_time64(io_context_t ctx, long min_nr, long max_nr,
		struct io_event *events, void *timeout, sigset_t *sigmask)
{
	return tst_syscall(__NR_io_pgetevents_time64, ctx, min_nr, max_nr,
			   events, timeout, sigmask);
}

#endif /* HAVE_LIBAIO */

#endif /* LAPI_IO_PGETEVENTS_H__ */
