// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Linaro Limited. All rights reserved.
 * Author: Viresh Kumar <viresh.kumar@linaro.org>
 */

#ifndef IO_PGETEVENTS_H
#define IO_PGETEVENTS_H

#include <sys/syscall.h>
#include <sys/types.h>

#include "config.h"
#include "lapi/syscalls.h"

#ifdef HAVE_LIBAIO
#include <libaio.h>

#ifndef HAVE_IO_PGETEVENTS
int io_pgetevents(io_context_t ctx, long min_nr, long max_nr,
		 struct io_event *events, struct timespec *timeout,
		 sigset_t *sigmask)
{
	return tst_syscall(__NR_io_pgetevents, ctx, min_nr, max_nr, events,
			   timeout, sigmask);
}
#endif /* HAVE_IO_PGETEVENTS */
#endif /* HAVE_LIBAIO */

#endif /* IO_PGETEVENTS_H */
