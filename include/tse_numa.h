/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TSE_NUMA_H__
#define TSE_NUMA_H__

#include <string.h>

/**
 * struct tse_nodemap - Numa nodemap.
 *
 * @cnt: Number of nodes in map.
 * @counters: Page allocation counters.
 * @map: Array of numa ids.
 */
struct tse_nodemap {
	unsigned int cnt;
	unsigned int *counters;
	unsigned int map[];
};

/**
 * tse_nodemap_reset_counters() - Clears numa counters. The counters are lazy-allocated on first call of this function.
 *
 * @nodes: Numa nodemap.
 */
void tse_nodemap_reset_counters(struct tse_nodemap *nodes);

/**
 * tse_nodemap_print_counters() - Prints pages allocated per each node.
 *
 * @nodes: Numa nodemap.
 */
void tse_nodemap_print_counters(struct tse_nodemap *nodes);

/**
 * tse_mempolicy_mode_name() - Returns a name for a mempolicy/mbind mode.
 *
 * @mode: Numa mempolicy mode.
 *
 * return: a name for a mempolicy/mbind mode.
 */
const char *tse_mempolicy_mode_name(int mode);

/**
 * tse_numa_map() - Maps pages into memory, if path is NULL the mapping is anonymous otherwise is backed by the file.
 *
 * @path: Path to a file, if not NULL mapping is file based.
 * @size: Mapping size.
 *
 * return: a pointer to a mapped file.
 */
void *tse_numa_map(const char *path, size_t size);

/**
 * tse_numa_fault() - Writes to memory in order to get the pages faulted.
 *
 * @ptr: Start of the mapping.
 * @size: Size of the mapping.
 */
static inline void tse_numa_fault(void *ptr, size_t size)
{
	memset(ptr, 'a', size);
}

/**
 * tse_numa_unmap() - Frees the memory.
 *
 * @ptr: Start of the mapping.
 * @size: Size of the mapping.
 */
static inline void tse_numa_unmap(void *ptr, size_t size)
{
	SAFE_MUNMAP(ptr, size);
}

/**
 * tse_nodemap_count_pages() - Check which numa node resides each page.
 *
 * Check on which numa node resides each page of the mapping starting at ptr
 * and continuing pages long and increases nodemap counters accordingly.
 *
 * @nodes: Nodemap with initialized counters.
 * @ptr:   Pointer to start of a mapping.
 * @size:  Size of the mapping.
 */
void tse_nodemap_count_pages(struct tse_nodemap *nodes, void *ptr, size_t size);

/**
 * tse_nodemap_free() - Frees nodemap.
 *
 * @nodes: Numa nodemap to be freed.
 */
void tse_nodemap_free(struct tse_nodemap *nodes);

/**
 * enum tse_numa_types - Bitflags for tse_get_nodemap() function.
 *
 * @TST_NUMA_ANY: general NUMA node.
 * @TST_NUMA_MEM: NUMA memory node.
 */
enum tse_numa_types {
	TST_NUMA_ANY = 0x00,
	TST_NUMA_MEM = 0x01,
};

/**
 * tse_get_nodemap() - Allocates and returns numa node map, which is an array of numa nodes which
 * contain desired resources e.g. memory.
 *
 * @type:       Bitflags of enum tst_numa_types specifying desired resources.
 * @min_mem_kb: Minimal free RAM on memory nodes, if given node has less than
 *             requested amount of free+buffers memory it's not included in
 *             the resulting list of nodes.
 *
 * return: On success returns allocated and initialized struct tse_nodemap which contains
 *         array of numa node ids that contains desired resources.
 */
struct tse_nodemap *tse_get_nodemap(int type, size_t min_mem_kb);

#endif /* TSE_NUMA_H__ */
