// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "config.h"
#ifdef HAVE_NUMA_V2
# include <numa.h>
# include <numaif.h>
#endif

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tse_numa.h"
#include "lapi/numaif.h"

void tse_nodemap_print_counters(struct tse_nodemap *nodes)
{
	unsigned int i;

	for (i = 0; i < nodes->cnt; i++) {
		tst_res(TINFO, "Node %i allocated %u pages",
		        nodes->map[i], nodes->counters[i]);
	}
}

void tse_nodemap_reset_counters(struct tse_nodemap *nodes)
{
	size_t arr_size = sizeof(unsigned int) * nodes->cnt;

	if (!nodes->counters)
		nodes->counters = SAFE_MALLOC(arr_size);

	memset(nodes->counters, 0, arr_size);
}

void tse_nodemap_free(struct tse_nodemap *nodes)
{
	free(nodes->counters);
	free(nodes);
}

#ifdef HAVE_NUMA_V2

const char *tse_mempolicy_mode_name(int mode)
{
	switch (mode) {
	case MPOL_DEFAULT:
		return "MPOL_DEFAULT";
	case MPOL_PREFERRED:
		return "MPOL_PREFERRED";
	case MPOL_BIND:
		return "MPOL_BIND";
	case MPOL_INTERLEAVE:
		return "MPOL_INTERLEAVE";
	case MPOL_LOCAL:
		return "MPOL_LOCAL";
	default:
		return "???";
	}
}


static void inc_counter(unsigned int node, struct tse_nodemap *nodes)
{
	unsigned int i;

	for (i = 0; i < nodes->cnt; i++) {
		if (nodes->map[i] == node) {
			nodes->counters[i]++;
			break;
		}
	}
}

void tse_nodemap_count_pages(struct tse_nodemap *nodes,
                             void *ptr, size_t size)
{
	size_t page_size = getpagesize();
	unsigned int i;
	int node;
	long ret;
	unsigned int pages = (size + page_size - 1)/page_size;

	for (i = 0; i < pages; i++) {
		ret = get_mempolicy(&node, NULL, 0, ptr + i * page_size, MPOL_F_NODE | MPOL_F_ADDR);
		if (ret < 0)
			tst_brk(TBROK | TERRNO, "get_mempolicy() failed");

		if (node < 0) {
			tst_res(TWARN,
				"get_mempolicy(...) returned invalid node %i\n", node);
			continue;
		}

		inc_counter(node, nodes);
	}
}

void *tse_numa_map(const char *path, size_t size)
{
	char *ptr;
	int fd = -1;
	int flags = MAP_PRIVATE|MAP_ANONYMOUS;

	if (path) {
		fd = SAFE_OPEN(path, O_CREAT | O_EXCL | O_RDWR, 0666);
		SAFE_FTRUNCATE(fd, size);
		flags = MAP_SHARED;
	}

	ptr = SAFE_MMAP(NULL, size,
	                PROT_READ|PROT_WRITE, flags, fd, 0);

	if (path) {
		SAFE_CLOSE(fd);
		SAFE_UNLINK(path);
	}

	return ptr;
}

static int node_has_enough_memory(int node, size_t min_kb)
{
	char path[1024];
	char buf[1024];
	long mem_total = -1;
	long mem_used = -1;
	long file_pages = 0;
	long mem_avail;

	/* Make sure there is some space for kernel upkeeping as well */
	min_kb += 4096;

	snprintf(path, sizeof(path), "/sys/devices/system/node/node%i/meminfo", node);

	if (access(path, F_OK)) {
		tst_res(TINFO, "File '%s' does not exist! NUMA not enabled?", path);
		return 0;
	}

	FILE *fp = fopen(path, "r");
	if (!fp)
		tst_brk(TBROK | TERRNO, "Failed to open '%s'", path);

	while (fgets(buf, sizeof(buf), fp)) {
		long val;

		if (sscanf(buf, "%*s %*i MemTotal: %li", &val) == 1)
			mem_total = val;

		if (sscanf(buf, "%*s %*i MemUsed: %li", &val) == 1)
			mem_used = val;

		if (sscanf(buf, "%*s %*i FilePages: %li", &val) == 1)
			file_pages = val;
	}

	fclose(fp);

	if (mem_total == -1 || mem_used == -1) {
		tst_res(TWARN, "Failed to parse '%s'", path);
		return 0;
	}

	mem_avail = mem_total - mem_used + (9 * file_pages)/10;

	if (mem_avail < (long)min_kb) {
		tst_res(TINFO,
		        "Not enough free RAM on node %i, have %likB needs %zukB",
		        node, mem_avail, min_kb);
		return 0;
	}

	return 1;
}

struct tse_nodemap *tse_get_nodemap(int type, size_t min_mem_kb)
{
	struct bitmask *membind;
	struct tse_nodemap *nodes;
	unsigned int i, cnt;

	if (type & ~(TST_NUMA_MEM))
		tst_brk(TBROK, "Invalid type %i\n", type);

	membind = numa_get_membind();

	cnt = 0;
	for (i = 0; i < membind->size; i++) {
		if (type & TST_NUMA_MEM && !numa_bitmask_isbitset(membind, i))
			continue;

		cnt++;
	}

	tst_res(TINFO, "Found %u NUMA memory nodes", cnt);

	nodes = SAFE_MALLOC(sizeof(struct tse_nodemap)
	                    + sizeof(unsigned int) * cnt);
	nodes->cnt = cnt;
	nodes->counters = NULL;

	cnt = 0;
	for (i = 0; i < membind->size; i++) {
		if (type & TST_NUMA_MEM &&
		    (!numa_bitmask_isbitset(membind, i) ||
		     !node_has_enough_memory(i, min_mem_kb)))
			continue;

		nodes->map[cnt++] = i;
	}

	nodes->cnt = cnt;

	numa_bitmask_free(membind);

	return nodes;
}

#endif
