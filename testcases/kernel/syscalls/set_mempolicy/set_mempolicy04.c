// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * We are testing set_mempolicy() with MPOL_INTERLEAVE on mmaped buffers backed
 * by files.
 *
 * Apparently it takes a larger sample for the allocations to be correctly
 * interleaved. The reason for this is that buffers for file metadata are
 * allocated in batches in order not to loose performance. Also the pages
 * cannot be interleaved completely evenly unless the number of pages is
 * divideable by the number of nodes, which will not happen even if we tried
 * hard since we do not have controll over metadata blocks for instance. Hence
 * we cannot really expect to allocate a single file and have the memory
 * interleaved precisely but it works well if we allocate statistic for a more
 * than a few files.
 */

#include <stdio.h>
#include <errno.h>
#include "config.h"
#ifdef HAVE_NUMA_V2
# include <numa.h>
# include <numaif.h>
#endif
#include "tst_test.h"
#include "tst_numa.h"

#define MNTPOINT "mntpoint"
#define FILES 10

#ifdef HAVE_NUMA_V2

#include "set_mempolicy.h"

static size_t page_size;
static struct tst_nodemap *nodes;

static void setup(void)
{
	page_size = getpagesize();

	nodes = tst_get_nodemap(TST_NUMA_MEM, 20 * FILES * page_size / 1024);
	if (nodes->cnt <= 1)
		tst_brk(TCONF, "Test requires at least two NUMA memory nodes");
}

static void cleanup(void)
{
	tst_nodemap_free(nodes);
}

static void alloc_and_check(void)
{
	unsigned int i, j;
	char path[1024];
	unsigned int total_pages = 0;
	unsigned int sum_pages = 0;

	tst_nodemap_reset_counters(nodes);

	/*
	 * The inner loop loops node->cnt times to ensure the sum could
	 * be evenly distributed among the nodes.
	 */
	for (i = 1; i <= FILES; i++) {
		for (j = 1; j <= nodes->cnt; j++) {
			size_t size = 10 * i + j % 10;
			snprintf(path, sizeof(path), MNTPOINT "/numa-test-file-%i-%i", i, j);
			alloc_fault_count(nodes, path, size * page_size);
			total_pages += size;
		}
	}

	for (i = 0; i < nodes->cnt; i++) {
		float treshold = 1.00 * total_pages / 60; /* five percents */
		float min_pages = 1.00 * total_pages / nodes->cnt - treshold;
		float max_pages = 1.00 * total_pages / nodes->cnt + treshold;

		if (nodes->counters[i] > min_pages && nodes->counters[i] < max_pages) {
			tst_res(TPASS, "Node %u allocated %u <%.2f,%.2f>",
			        nodes->map[i], nodes->counters[i], min_pages, max_pages);
		} else {
			tst_res(TFAIL, "Node %u allocated %u, expected <%.2f,%.2f>",
			        nodes->map[i], nodes->counters[i], min_pages, max_pages);
		}

		sum_pages += nodes->counters[i];
	}

	if (sum_pages != total_pages) {
		tst_res(TFAIL, "Sum of nodes %u != allocated pages %u",
		        sum_pages, total_pages);
		return;
	}

	tst_res(TPASS, "Sum of nodes equals to allocated pages (%u)", total_pages);
}

static void verify_set_mempolicy(void)
{
	struct bitmask *bm = numa_allocate_nodemask();
	unsigned int alloc_on_nodes = nodes->cnt;
	unsigned int i;

	for (i = 0; i < alloc_on_nodes; i++)
		numa_bitmask_setbit(bm, nodes->map[i]);

	TEST(set_mempolicy(MPOL_INTERLEAVE, bm->maskp, bm->size+1));

	if (TST_RET) {
		tst_res(TFAIL | TTERRNO,
		        "set_mempolicy(MPOL_INTERLEAVE)");
		return;
	}

	tst_res(TPASS, "set_mempolicy(MPOL_INTERLEAVE)");

	alloc_and_check();

	numa_free_nodemask(bm);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_set_mempolicy,
	.forks_child = 1,
	.needs_root = 1,
	.all_filesystems = 1,
	.mntpoint = MNTPOINT,
	.needs_checkpoints = 1,
};

#else

TST_TEST_TCONF(NUMA_ERROR_MSG);

#endif /* HAVE_NUMA_V2 */
