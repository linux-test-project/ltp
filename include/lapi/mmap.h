// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015 Fujitsu Ltd.
 * Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 */

#ifndef LAPI_MMAP_H__
#define LAPI_MMAP_H__

#include "config.h"
#include <sys/mman.h>

#ifndef MAP_HUGETLB
# define MAP_HUGETLB 0x40000
#endif

#ifndef MADV_REMOVE
# define MADV_REMOVE 9
#endif

#ifndef MADV_DONTFORK
# define MADV_DONTFORK 10
#endif

#ifndef MADV_DOFORK
# define MADV_DOFORK   11
#endif

#ifndef MADV_HWPOISON
# define MADV_HWPOISON 100
#endif

#ifndef MADV_SOFT_OFFLINE
# define MADV_SOFT_OFFLINE 101
#endif

#ifndef MADV_MERGEABLE
# define MADV_MERGEABLE   12
#endif

#ifndef MADV_UNMERGEABLE
# define MADV_UNMERGEABLE 13
#endif

#ifndef MADV_HUGEPAGE
# define MADV_HUGEPAGE   14
#endif

#ifndef MADV_NOHUGEPAGE
# define MADV_NOHUGEPAGE 15
#endif

#ifndef MADV_DONTDUMP
# define MADV_DONTDUMP 16
#endif

#ifndef MADV_DODUMP
# define MADV_DODUMP   17
#endif

#ifndef MADV_FREE
# define MADV_FREE	8
#endif

#ifndef MADV_WIPEONFORK
# define MADV_WIPEONFORK 18
# define MADV_KEEPONFORK 19
#endif

#ifndef MADV_COLD
# define MADV_COLD	20
#endif

#ifndef MADV_PAGEOUT
# define MADV_PAGEOUT	21
#endif

#ifndef MAP_FIXED_NOREPLACE

#ifdef __alpha__
# define MAP_FIXED_NOREPLACE 0x200000
#else
# define MAP_FIXED_NOREPLACE 0x100000
#endif

#endif /* MAP_FIXED_NOREPLACE */

#ifdef HAVE_SYS_SHM_H
# include <sys/shm.h>
# define MMAP_GRANULARITY SHMLBA
#else
# include <unistd.h>
# define MMAP_GRANULARITY getpagesize()
#endif /* HAVE_SYS_SHM_H */

#endif /* LAPI_MMAP_H__ */
