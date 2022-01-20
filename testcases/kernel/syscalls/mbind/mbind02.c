// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * We are testing mbind() EIO error.
 *
 * We first fault a allocated page, then attempt to mbind it to a different node.
 *
 * This is a regression test for:
 *
 * a7f40cfe3b7a mm: mempolicy: make mbind() return -EIO when MPOL_MF_STRICT is specified
 *
 */

#include <errno.h>
#include "config.h"
#ifdef HAVE_NUMA_H
# include <numa.h>
# include <numaif.h>
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

static void verify_policy(int mode)
{
	struct bitmask *bm = numa_allocate_nodemask();
	unsigned int i;
	void *ptr;
	unsigned long size = page_size;
	int node = 0;

	ptr = tst_numa_map(NULL, size);
	tst_nodemap_reset_counters(nodes);
	tst_numa_fault(ptr, size);
	tst_nodemap_count_pages(nodes, ptr, size);
	tst_nodemap_print_counters(nodes);

	for (i = 0; i < nodes->cnt; i++) {
		if (!nodes->counters[i]) {
			node = nodes->map[i];
			tst_res(TINFO, "Attempting to bind to node %i", node);
			numa_bitmask_setbit(bm, node);
			break;
		}
	}

	TEST(mbind(ptr, size, mode, bm->maskp, bm->size + 1, MPOL_MF_STRICT));

	tst_numa_unmap(ptr, size);
	numa_free_nodemask(bm);

	if (TST_RET != -1) {
		tst_res(TFAIL,
		        "mbind(%s, MPOL_MF_STRICT) node %u returned %li, expected -1",
		        tst_mempolicy_mode_name(mode), node, TST_RET);
		return;
	}

	if (TST_ERR == EIO) {
		tst_res(TPASS | TTERRNO,
		        "mbind(%s, MPOL_MF_STRICT) node %u",
		        tst_mempolicy_mode_name(mode), node);
	} else {
		tst_res(TFAIL | TTERRNO,
			"mbind(%s, MPOL_MF_STRICT) node %u expected EIO",
		        tst_mempolicy_mode_name(mode), node);
	}
}

static const int modes[] = {
	MPOL_PREFERRED,
	MPOL_BIND,
	MPOL_INTERLEAVE,
};

static void verify_mbind(unsigned int n)
{
	verify_policy(modes[n]);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_mbind,
	.tcnt = ARRAY_SIZE(modes),
	.tags = (const struct tst_tag[]) {
		{"linux-git", "a7f40cfe3b7a"},
		{}
	}
};

#else

TST_TEST_TCONF(NUMA_ERROR_MSG);

#endif /* HAVE_NUMA_H */
