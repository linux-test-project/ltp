// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2013-2017  Red Hat, Inc.
 */
/*\
 * [Description]
 *
 * The case is designed to test sysfs boolean knob
 * /sys/kernel/mm/ksm/merge_across_nodes.
 *
 * When merge_across_nodes is set to zero only pages from the same
 * node are merged, otherwise pages from all nodes can be merged
 * together.
 *
 * Introduced in commit:
 *
 *  commit 90bd6fd31c8097ee4ddcb74b7e08363134863de5
 *   Author: Petr Holasek <pholasek@redhat.com>
 *   Date:   Fri Feb 22 16:35:00 2013 -0800
 *
 *   ksm: allow trees per NUMA node
 */

#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>

#include "mem.h"
#include "tst_numa.h"

#ifdef HAVE_NUMA_V2
# include <numa.h>
# include <numaif.h>

static unsigned long nr_pages = 100;
static char *n_opt;

static size_t page_size;
static struct tst_nodemap *nodes;

static void test_ksm(void)
{
	char **memory;
	unsigned int i;
	int ret;
	unsigned long length;
	struct bitmask *bm = numa_allocate_nodemask();

	length = nr_pages * page_size;

	memory = SAFE_MALLOC(nodes->cnt * sizeof(char *));
	for (i = 0; i < nodes->cnt; i++) {
		memory[i] = SAFE_MMAP(NULL, length, PROT_READ|PROT_WRITE,
			    MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
#ifdef HAVE_DECL_MADV_MERGEABLE
		if (madvise(memory[i], length, MADV_MERGEABLE) == -1)
			tst_brk(TBROK|TERRNO, "madvise");
#endif

#ifdef HAVE_NUMA_V2
		numa_bitmask_setbit(bm, nodes->map[i]);

		ret = mbind(memory[i], length, MPOL_BIND, bm->maskp, bm->size+1, 0);
		if (ret == -1)
			tst_brk(TBROK|TERRNO, "mbind");

		numa_bitmask_clearbit(bm, nodes->map[i]);
#endif

		memset(memory[i], 10, length);

		if (mlock(memory[i], length))
			tst_res(TWARN | TERRNO, "mlock() failed");
	}

	numa_free_nodemask(bm);

	SAFE_FILE_PRINTF(PATH_KSM "sleep_millisecs", "0");
	SAFE_FILE_PRINTF(PATH_KSM "pages_to_scan", "%ld",
			 nr_pages * nodes->cnt);
	/*
	 * merge_across_nodes and max_page_sharing setting can be changed
	 * only when there are no ksm shared pages in system, so set run 2
	 * to unmerge pages first, then to 1 after changing merge_across_nodes,
	 * to remerge according to the new setting.
	 */
	SAFE_FILE_PRINTF(PATH_KSM "run", "2");
	if (access(PATH_KSM "max_page_sharing", F_OK) == 0)
		SAFE_FILE_PRINTF(PATH_KSM "max_page_sharing",
			"%ld", nr_pages * nodes->cnt);
	tst_res(TINFO, "Start to test KSM with merge_across_nodes=1");
	SAFE_FILE_PRINTF(PATH_KSM "merge_across_nodes", "1");
	SAFE_FILE_PRINTF(PATH_KSM "run", "1");
	ksm_group_check(1, 1, nr_pages * nodes->cnt - 1, 0, 0, 0,
			nr_pages * nodes->cnt);

	SAFE_FILE_PRINTF(PATH_KSM "run", "2");
	tst_res(TINFO, "Start to test KSM with merge_across_nodes=0");
	SAFE_FILE_PRINTF(PATH_KSM "merge_across_nodes", "0");
	SAFE_FILE_PRINTF(PATH_KSM "run", "1");
	ksm_group_check(1, nodes->cnt, nr_pages * nodes->cnt - nodes->cnt,
			0, 0, 0, nr_pages * nodes->cnt);

	SAFE_FILE_PRINTF(PATH_KSM "run", "2");

	for (i = 0; i < nodes->cnt; i++)
		SAFE_MUNMAP(memory[i], length);

	free(memory);
}

static void setup(void)
{
	if (n_opt)
		nr_pages = SAFE_STRTOUL(n_opt, 0, ULONG_MAX);

	page_size = getpagesize();

	nodes = tst_get_nodemap(TST_NUMA_MEM, nr_pages * page_size / 1024);
	if (nodes->cnt <= 1)
		tst_brk(TCONF, "Test requires at least two NUMA memory nodes");
}

static struct tst_test test = {
	.needs_root = 1,
	.options = (struct tst_option[]) {
		{"n:", &n_opt,  "Allocate x pages memory per node"},
		{}
	},
	.setup = setup,
	.save_restore = (const struct tst_path_val[]) {
		{"/sys/kernel/mm/ksm/max_page_sharing", NULL,
			TST_SR_SKIP_MISSING | TST_SR_TCONF_RO},
		{"/sys/kernel/mm/ksm/run", NULL, TST_SR_TBROK},
		{"/sys/kernel/mm/ksm/sleep_millisecs", NULL, TST_SR_TBROK},
		{"/sys/kernel/mm/ksm/merge_across_nodes", NULL, TST_SR_TCONF},
		{}
	},
	.needs_kconfigs = (const char *const[]){
		"CONFIG_KSM=y",
		"CONFIG_NUMA=y",
		NULL
	},
	.test_all = test_ksm,
};

#else
	TST_TEST_TCONF(NUMA_ERROR_MSG);
#endif
