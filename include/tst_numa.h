/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TST_NUMA_H__
#define TST_NUMA_H__

#include <string.h>

/**
 * Numa nodemap.
 */
struct tst_nodemap {
        /** Number of nodes in map */
	unsigned int cnt;
	/** Page allocation counters */
	unsigned int *counters;
	/** Array of numa ids */
	unsigned int map[];
};

/**
 * Clears numa counters. The counters are lazy-allocated on first call of this function.
 *
 * @nodes Numa nodemap.
 */
void tst_nodemap_reset_counters(struct tst_nodemap *nodes);

/**
 * Prints pages allocated per each node.
 *
 * @nodes Numa nodemap.
 */
void tst_nodemap_print_counters(struct tst_nodemap *nodes);

/**
 * Returns a name for a mempolicy/mbind mode.
 *
 * @mode Numa mempolicy mode.
 */
const char *tst_numa_mode_name(int mode);

/**
 * Maps pages into memory, if path is NULL the mapping is anonymous otherwise is backed by the file.
 *
 * @path Path to a file, if not NULL mapping is file based.
 * @size Mapping size.
 */
void *tst_numa_map(const char *path, size_t size);

/*
 * Writes to memory in order to get the pages faulted.
 *
 * @ptr Start of the mapping.
 * @size Size of the mapping.
 */
static inline void tst_numa_fault(void *ptr, size_t size)
{
	memset(ptr, 'a', size);
}

/*
 * Frees the memory.
 *
 * @ptr Start of the mapping.
 * @size Size of the mapping.
 */
static inline void tst_numa_unmap(void *ptr, size_t size)
{
	SAFE_MUNMAP(ptr, size);
}

/**
 * Check on which numa node resides each page of the mapping starting at ptr
 * and continuing pages long and increases nodemap counters accordingly.
 *
 * @nodes Nodemap with initialized counters.
 * @ptr   Pointer to start of a mapping.
 * @size  Size of the mapping.
 */
void tst_nodemap_count_pages(struct tst_nodemap *nodes, void *ptr, size_t size);

/**
 * Frees nodemap.
 *
 * @nodes Numa nodemap to be freed.
 */
void tst_nodemap_free(struct tst_nodemap *nodes);

/**
 * Bitflags for tst_get_nodemap() function.
 */
enum tst_numa_types {
	TST_NUMA_ANY = 0x00,
	TST_NUMA_MEM = 0x01,
};

/**
 * Allocates and returns numa node map, which is an array of numa nodes which
 * contain desired resources e.g. memory.
 *
 * @type       Bitflags of enum tst_numa_types specifying desired resources.
 * @min_mem_kb Minimal free RAM on memory nodes, if given node has less than
 *             requested amount of free+buffers memory it's not included in
 *             the resulting list of nodes.
 *
 * @return On success returns allocated and initialized struct tst_nodemap which contains
 *         array of numa node ids that contains desired resources.
 */
struct tst_nodemap *tst_get_nodemap(int type, size_t min_mem_kb);

#endif /* TST_NUMA_H__ */
