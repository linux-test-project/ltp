// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * We are testing mbind() with MPOL_BIND, MPOL_PREFERRED and MPOL_INTERLEAVE
 *
 * For each node with memory we set its bit in nodemask with set_mempolicy()
 * and verify that memory has been faulted accordingly.
 */

#include <errno.h>
#include "config.h"
#ifdef HAVE_NUMA_H
# include <numa.h>
# include <numaif.h>
# include "mbind.h"
#endif
#include "tst_test.h"
#include "tse_numa.h"

#ifdef HAVE_NUMA_V2

static size_t page_size;
static struct tse_nodemap *nodes;

#define PAGES_ALLOCATED 16u

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

static void verify_policy(unsigned int node, int mode, unsigned flag)
{
	struct bitmask *bm = numa_allocate_nodemask();
	unsigned int i;
	void *ptr;
	pid_t pid;
	unsigned long size = PAGES_ALLOCATED * page_size;

	numa_bitmask_setbit(bm, node);

	ptr = tse_numa_map(NULL, size);

	TEST(mbind(ptr, size, mode, bm->maskp, bm->size + 1, flag));

	numa_free_nodemask(bm);

	if (TST_RET) {
		tst_res(TFAIL | TTERRNO,
		        "mbind(%s, %s) node %u",
		        tse_mempolicy_mode_name(mode), mbind_flag_name(flag), node);
		return;
	}

	tst_res(TPASS, "mbind(%s, %s) node %u",
	        tse_mempolicy_mode_name(mode), mbind_flag_name(flag), node);

	const char *prefix = "child: ";

	pid = SAFE_FORK();
	if (pid) {
		prefix = "parent: ";
		tst_reap_children();
	}

	tse_nodemap_reset_counters(nodes);
	tse_numa_fault(ptr, size);
	tse_nodemap_count_pages(nodes, ptr, size);
	tse_numa_unmap(ptr, size);

	int fail = 0;

	for (i = 0; i < nodes->cnt; i++) {
		if (nodes->map[i] == node) {
			if (nodes->counters[i] == PAGES_ALLOCATED) {
				tst_res(TPASS, "%sNode %u allocated %u",
				        prefix, node, PAGES_ALLOCATED);
			} else {
				tst_res(TFAIL, "%sNode %u allocated %u, expected %u",
				        prefix, node, nodes->counters[i],
				        PAGES_ALLOCATED);
				fail = 1;
			}
			continue;
		}

		if (nodes->counters[i]) {
			tst_res(TFAIL, "%sNode %u allocated %u, expected 0",
			        prefix, i, nodes->counters[i]);
			fail = 1;
		}
	}

	if (fail)
		tse_nodemap_print_counters(nodes);

	if (!pid)
		exit(0);
}

static const int modes[] = {
	MPOL_PREFERRED,
	MPOL_BIND,
	MPOL_INTERLEAVE,
};

static void verify_mbind(unsigned int n)
{
	unsigned int i;

	for (i = 0; i < nodes->cnt; i++) {
		verify_policy(nodes->map[i], modes[n], 0);
		verify_policy(nodes->map[i], modes[n], MPOL_MF_STRICT);
	}
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_mbind,
	.tcnt = ARRAY_SIZE(modes),
	.forks_child = 1,
};

#else

TST_TEST_TCONF(NUMA_ERROR_MSG);

#endif /* HAVE_NUMA_H */
