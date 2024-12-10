// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef LAPI_MMAN_H__
#define LAPI_MMAN_H__

#include <stdint.h>
#include <sys/mman.h>
#include "config.h"
#include "lapi/syscalls.h"

#ifndef HAVE_STRUCT_CACHESTAT_RANGE
struct cachestat_range {
	uint64_t off;
	uint64_t len;
};
#endif

#ifndef HAVE_STRUCT_CACHESTAT
struct cachestat {
	uint64_t nr_cache;
	uint64_t nr_dirty;
	uint64_t nr_writeback;
	uint64_t nr_evicted;
	uint64_t nr_recently_evicted;
};
#endif

#ifndef HAVE_CACHESTAT
/*
 * cachestat: wrapper function of cachestat
 *
 * Returns: It returns status of cachestat syscall
 */
static inline int cachestat(int fd, struct cachestat_range *cstat_range,
	struct cachestat *cstat, unsigned int flags)
{
	return tst_syscall(__NR_cachestat, fd, cstat_range, cstat, flags);
}
#endif

#endif /* LAPI_MMAN_H__ */
