/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef SET_MEMPOLICY_H__
#define SET_MEMPOLICY_H__

static inline void alloc_fault_count(struct tse_nodemap *nodes,
                                     const char *file, size_t size)
{
	void *ptr;

	ptr = tse_numa_map(file, size);
	tse_numa_fault(ptr, size);
	tse_nodemap_count_pages(nodes, ptr, size);
	tse_numa_unmap(ptr, size);
}

#endif /* SET_MEMPOLICY_H__ */
