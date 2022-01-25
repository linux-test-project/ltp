// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015 Cui Bixuan <cuibixuan@huawei.com>
 */

#ifndef LAPI_SCHED_H__
#define LAPI_SCHED_H__

#include <sched.h>
#include <stdint.h>
#include <inttypes.h>
#include "lapi/syscalls.h"

struct sched_attr {
	uint32_t size;

	uint32_t sched_policy;
	uint64_t sched_flags;

	/* SCHED_NORMAL, SCHED_BATCH */
	int32_t sched_nice;

	/* SCHED_FIFO, SCHED_RR */
	uint32_t sched_priority;

	/* SCHED_DEADLINE (nsec) */
	uint64_t sched_runtime;
	uint64_t sched_deadline;
	uint64_t sched_period;
};

static inline int sched_setattr(pid_t pid, const struct sched_attr *attr,
                                unsigned int flags)
{
	return syscall(__NR_sched_setattr, pid, attr, flags);
}

static inline int sched_getattr(pid_t pid, struct sched_attr *attr,
                                unsigned int size, unsigned int flags)
{
	return syscall(__NR_sched_getattr, pid, attr, size, flags);
}

#ifndef CLONE_VM
#define CLONE_VM   0x00000100
#endif

#ifndef CLONE_FS
#define CLONE_FS   0x00000200
#endif

#ifndef CLONE_SYSVSEM
#define CLONE_SYSVSEM   0x00040000
#endif

#ifndef CLONE_IO
#define CLONE_IO        0x80000000
#endif

#endif /* LAPI_SCHED_H__ */
