// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * We are testing set_mempolicy() with MPOL_BIND and MPOL_PREFERRED backed by a
 * file on each supported filesystem.
 */

#include <errno.h>
#include "config.h"
#ifdef HAVE_NUMA_V2
# include <numaif.h>
# include <numa.h>
#endif
#include "tst_test.h"
#include "tse_numa.h"

#define MNTPOINT "mntpoint"
#define PAGES_ALLOCATED 16u

#ifdef HAVE_NUMA_V2

#include "set_mempolicy.h"

static size_t page_size;
static struct tse_nodemap *nodes;

static void setup(void)
{
	page_size = getpagesize();

	nodes = tse_get_nodemap(TST_NUMA_MEM, 2 * PAGES_ALLOCATED * page_size / 1024);
	if (nodes->cnt <= 1)
		tst_brk(TCONF, "Test requires at least two NUMA memory nodes");
}

static void cleanup(void)
{
	tse_nodemap_free(nodes);
}

static void verify_mempolicy(unsigned int node, int mode)
{
	struct bitmask *bm = numa_allocate_nodemask();
	unsigned int i;

	numa_bitmask_setbit(bm, node);

	TEST(set_mempolicy(mode, bm->maskp, bm->size+1));

	if (TST_RET) {
		tst_res(TFAIL | TTERRNO,
		        "set_mempolicy(%s) node %u",
		        tse_mempolicy_mode_name(mode), node);
		return;
	}

	tst_res(TPASS, "set_mempolicy(%s) node %u",
	        tse_mempolicy_mode_name(mode), node);

	numa_free_nodemask(bm);

	tse_nodemap_reset_counters(nodes);
	alloc_fault_count(nodes, MNTPOINT "/numa-test-file", PAGES_ALLOCATED * page_size);

	for (i = 0; i < nodes->cnt; i++) {
		if (nodes->map[i] == node) {
			if (nodes->counters[i] == PAGES_ALLOCATED) {
				tst_res(TPASS, "Node %u allocated %u",
				        node, PAGES_ALLOCATED);
			} else {
				tst_res(TFAIL, "Node %u allocated %u, expected %u",
				        node, nodes->counters[i], PAGES_ALLOCATED);
			}
			continue;
		}

		if (nodes->counters[i]) {
			tst_res(TFAIL, "Node %u allocated %u, expected 0",
			        node, nodes->counters[i]);
		}
	}
}

static void verify_set_mempolicy(unsigned int n)
{
	unsigned int i;
	int mode = n ? MPOL_PREFERRED : MPOL_BIND;

	for (i = 0; i < nodes->cnt; i++)
		verify_mempolicy(nodes->map[i], mode);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_set_mempolicy,
	.tcnt = 2,
	.needs_root = 1,
	.all_filesystems = 1,
	.mntpoint = MNTPOINT,
	.forks_child = 1,
	.needs_checkpoints = 1,
};

#else

TST_TEST_TCONF(NUMA_ERROR_MSG);

#endif /* HAVE_NUMA_V2 */
