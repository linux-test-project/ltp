/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (c) 2019 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * We are testing mbind() MPOL_MF_MOVE and MPOL_MF_MOVE_ALL.
 *
 * If one of these flags is passed along with the policy kernel attempts to
 * move already faulted pages to match the requested policy.
 */

#include <errno.h>
#include "config.h"
#ifdef HAVE_NUMA_H
# include <numa.h>
# include <numaif.h>
# include "mbind.h"
#endif
#include "tst_test.h"
#include "tst_numa.h"

#ifdef HAVE_NUMA_V2

static size_t page_size;
static struct tst_nodemap *nodes;

static void setup(void)
{
	page_size = getpagesize();

	nodes = tst_get_nodemap(TST_NUMA_MEM, 2 * page_size / 1024);
	if (nodes->cnt <= 1)
		tst_brk(TCONF, "Test requires at least two NUMA memory nodes");
}

static void cleanup(void)
{
	tst_nodemap_free(nodes);
}

static void verify_policy(int mode, unsigned flag)
{
	struct bitmask *bm = numa_allocate_nodemask();
	unsigned int i;
	void *ptr;
	unsigned long size = page_size;
	unsigned int node = 0;

	ptr = tst_numa_map(NULL, size);
	tst_nodemap_reset_counters(nodes);
	tst_numa_fault(ptr, size);
	tst_nodemap_count_pages(nodes, ptr, size);
	tst_nodemap_print_counters(nodes);

	for (i = 0; i < nodes->cnt; i++) {
		if (!nodes->counters[i]) {
			node = nodes->map[i];
			tst_res(TINFO, "Attempting to move to node %i", node);
			numa_bitmask_setbit(bm, node);
			break;
		}
	}

	TEST(mbind(ptr, size, mode, bm->maskp, bm->size + 1, flag));

	if (TST_RET) {
		tst_res(TFAIL | TTERRNO,
		        "mbind(%s, %s) node %u",
		        tst_numa_mode_name(mode), mbind_flag_name(flag), node);
		goto exit;
	} else {
		tst_res(TPASS, "mbind(%s, %s) node %u succeded",
		        tst_numa_mode_name(mode), mbind_flag_name(flag), node);
	}

	tst_nodemap_reset_counters(nodes);
	tst_nodemap_count_pages(nodes, ptr, size);

	for (i = 0; i < nodes->cnt; i++) {
		if (nodes->map[i] == node) {
			if (nodes->counters[i] == 1) {
				tst_res(TPASS, "Node %u allocated %u", node, 1);
			} else {
				tst_res(TFAIL, "Node %u allocated %u, expected %u",
				        node, nodes->counters[i], 0);
			}
			continue;
		}

		if (nodes->counters[i]) {
			tst_res(TFAIL, "Node %u allocated %u, expected 0",
			        i, nodes->counters[i]);
		}
	}

exit:
	tst_numa_unmap(ptr, size);
	numa_free_nodemask(bm);
}

static const int modes[] = {
	MPOL_PREFERRED,
	MPOL_BIND,
	MPOL_INTERLEAVE,
};

static void verify_mbind(unsigned int n)
{
	verify_policy(modes[n], MPOL_MF_MOVE);
	verify_policy(modes[n], MPOL_MF_MOVE_ALL);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_mbind,
	.tcnt = ARRAY_SIZE(modes),
	.needs_root = 1,
};

#else

TST_TEST_TCONF(NUMA_ERROR_MSG);

#endif /* HAVE_NUMA_H */
