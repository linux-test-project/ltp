/*
 * Copyright (c) 2015 Fujitsu Ltd.
 * Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
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
 * along with this program.
 */

#ifndef LAPI_MMAP_H__
#define LAPI_MMAP_H__

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

#endif /* LAPI_MMAP_H__ */
