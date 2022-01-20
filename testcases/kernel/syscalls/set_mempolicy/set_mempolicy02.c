// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * We are testing set_mempolicy() with MPOL_INTERLEAVE.
 *
 * The test tries different subsets of memory nodes, sets the mask with
 * memopolicy, and checks that the memory was interleaved between the nodes
 * accordingly.
 */

#include <errno.h>
#include "config.h"
#ifdef HAVE_NUMA_V2
# include <numa.h>
# include <numaif.h>
#endif
#include "tst_test.h"
#include "tst_numa.h"

#ifdef HAVE_NUMA_V2

#include "set_mempolicy.h"

#define ALLOC_ON_NODE 8

static size_t page_size;
static struct tst_nodemap *nodes;

static void setup(void)
{
	page_size = getpagesize();

	nodes = tst_get_nodemap(TST_NUMA_MEM, 2 * ALLOC_ON_NODE * page_size / 1024);
	if (nodes->cnt <= 1)
		tst_brk(TCONF, "Test requires at least two NUMA memory nodes");
}

static void cleanup(void)
{
	tst_nodemap_free(nodes);
}

static void alloc_and_check(size_t size, unsigned int *exp_alloc)
{
	unsigned int i;
	const char *prefix = "child: ";

	if (SAFE_FORK()) {
		prefix = "parent: ";
		tst_reap_children();
	}

	tst_nodemap_reset_counters(nodes);
	alloc_fault_count(nodes, NULL, size * page_size);

	for (i = 0; i < nodes->cnt; i++) {
		if (nodes->counters[i] == exp_alloc[i]) {
			tst_res(TPASS, "%sNode %u allocated %u",
			        prefix, nodes->map[i], exp_alloc[i]);
		} else {
			tst_res(TFAIL, "%sNode %u allocated %u, expected %u",
			        prefix, nodes->map[i], nodes->counters[i],
			        exp_alloc[i]);
		}
	}
}

static void verify_set_mempolicy(unsigned int n)
{
	struct bitmask *bm = numa_allocate_nodemask();
	unsigned int exp_alloc[nodes->cnt];
	unsigned int alloc_per_node = n ? ALLOC_ON_NODE : 2;
	unsigned int alloc_on_nodes = n ? 2 : nodes->cnt;
	unsigned int alloc_total = alloc_per_node * alloc_on_nodes;
	unsigned int i;

	memset(exp_alloc, 0, sizeof(exp_alloc));

	for (i = 0; i < alloc_on_nodes; i++) {
		exp_alloc[i] = alloc_per_node;
		numa_bitmask_setbit(bm, nodes->map[i]);
	}

	TEST(set_mempolicy(MPOL_INTERLEAVE, bm->maskp, bm->size+1));

	tst_res(TINFO, "Allocating on nodes 1-%u - %u pages",
	        alloc_on_nodes, alloc_total);

	if (TST_RET) {
		tst_res(TFAIL | TTERRNO,
		        "set_mempolicy(MPOL_INTERLEAVE)");
		return;
	}

	tst_res(TPASS, "set_mempolicy(MPOL_INTERLEAVE)");

	numa_free_nodemask(bm);

	alloc_and_check(alloc_total, exp_alloc);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_set_mempolicy,
	.tcnt = 2,
	.forks_child = 1,
	.needs_checkpoints = 1,
};

#else

TST_TEST_TCONF(NUMA_ERROR_MSG);

#endif /* HAVE_NUMA_V2 */
