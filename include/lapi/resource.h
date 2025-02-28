// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Red Hat Inc. All Rights Reserved.
 * Author: Chunfu Wen <chwen@redhat.com>
 */

#ifndef LAPI_RESOURCE_H__
#define LAPI_RESOURCE_H__

#define _LARGEFILE64_SOURCE

#include <sys/resource.h>
#include "config.h"
#include "lapi/syscalls.h"

#ifndef HAVE_STRUCT_RLIMIT64
struct rlimit64 {
        uint64_t rlim_cur;
        uint64_t rlim_max;
};
#endif

static int setrlimit_u64(int resource, struct rlimit64 *rlim)
{
        return tst_syscall(__NR_prlimit64, 0, resource, rlim, NULL);
}

#endif /* LAPI_RESOURCE_H__ */
