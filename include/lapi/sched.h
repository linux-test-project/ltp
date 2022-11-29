// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015 Cui Bixuan <cuibixuan@huawei.com>
 * Copyright (c) Linux Test Project, 2016-2022
 */

#ifndef LAPI_SCHED_H__
#define LAPI_SCHED_H__

#include <sched.h>
#include <unistd.h>
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

#ifndef SCHED_DEADLINE
# define SCHED_DEADLINE	6
#endif

#ifndef CLONE_VM
# define CLONE_VM	0x00000100
#endif

#ifndef CLONE_FS
# define CLONE_FS	0x00000200
#endif

#ifndef CLONE_NEWNS
# define CLONE_NEWNS	0x00020000
#endif

#ifndef CLONE_SYSVSEM
# define CLONE_SYSVSEM	0x00040000
#endif

#ifndef CLONE_NEWCGROUP
# define CLONE_NEWCGROUP	0x02000000
#endif

#ifndef CLONE_NEWUTS
# define CLONE_NEWUTS		0x04000000
#endif

#ifndef CLONE_NEWIPC
#  define CLONE_NEWIPC		0x08000000
#endif

#ifndef CLONE_NEWUSER
#  define CLONE_NEWUSER		0x10000000
#endif

#ifndef CLONE_NEWPID
#  define CLONE_NEWPID		0x20000000
#endif

#ifndef CLONE_NEWNET
# define CLONE_NEWNET		0x40000000
#endif

#ifndef CLONE_IO
# define CLONE_IO		0x80000000
#endif

#ifndef CLONE_NEWTIME
# define CLONE_NEWTIME		0x00000080
#endif

#endif /* LAPI_SCHED_H__ */
