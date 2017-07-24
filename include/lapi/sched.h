/*
 * Copyright (c) 2015 Cui Bixuan <cuibixuan@huawei.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include "lapi/syscalls.h"
#include <stdint.h>
#include <inttypes.h>

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

int sched_setattr(pid_t pid,
	const struct sched_attr *attr,
	unsigned int flags)
{
	return syscall(__NR_sched_setattr, pid, attr, flags);
}

int sched_getattr(pid_t pid,
	struct sched_attr *attr,
	unsigned int size,
	unsigned int flags)
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

#endif /* __SCHED_H__ */
