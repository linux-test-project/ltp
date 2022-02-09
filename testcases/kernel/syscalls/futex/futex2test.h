/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Futex2 library addons for futex tests
 *
 * Copyright 2021 Collabora Ltd.
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef FUTEX2TEST_H
#define FUTEX2TEST_H

#include <stdint.h>
#include "lapi/syscalls.h"
#include "futextest.h"

/**
 * futex_waitv - Wait at multiple futexes, wake on any
 * @waiters:    Array of waiters
 * @nr_waiters: Length of waiters array
 * @flags: Operation flags
 * @timo:  Optional timeout for operation
 */
static inline int futex_waitv(volatile struct futex_waitv *waiters,
			      unsigned long nr_waiters, unsigned long flags,
			      struct timespec *timo, clockid_t clockid)
{
	return tst_syscall(__NR_futex_waitv, waiters, nr_waiters, flags, timo, clockid);
}

#endif /* _FUTEX2TEST_H */
