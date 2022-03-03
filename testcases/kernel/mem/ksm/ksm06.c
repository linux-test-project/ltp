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
#include "numa_helper.h"

#ifdef HAVE_NUMA_V2
#include <numaif.h>

static int run = -1;
static int sleep_millisecs = -1;
static int merge_across_nodes = -1;
static unsigned long nr_pages = 100;

static char *n_opt;

static void test_ksm(void)
{
	char **memory;
	int i, ret;
	int num_nodes, *nodes;
	unsigned long length;
	unsigned long pagesize;

#ifdef HAVE_NUMA_V2
	unsigned long nmask[MAXNODES / BITS_PER_LONG] = { 0 };
#endif

	ret = get_allowed_nodes_arr(NH_MEMS, &num_nodes, &nodes);
	if (ret != 0)
		tst_brk(TBROK|TERRNO, "get_allowed_nodes_arr");
	if (num_nodes < 2) {
		tst_res(TINFO, "need NUMA system support");
		free(nodes);
		return;
	}

	pagesize = sysconf(_SC_PAGE_SIZE);
	length = nr_pages * pagesize;

	memory = SAFE_MALLOC(num_nodes * sizeof(char *));
	for (i = 0; i < num_nodes; i++) {
		memory[i] = SAFE_MMAP(NULL, length, PROT_READ|PROT_WRITE,
			    MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
#ifdef HAVE_DECL_MADV_MERGEABLE
		if (madvise(memory[i], length, MADV_MERGEABLE) == -1)
			tst_brk(TBROK|TERRNO, "madvise");
#endif

#ifdef HAVE_NUMA_V2
		clean_node(nmask);
		set_node(nmask, nodes[i]);
		/*
		 * Use mbind() to make sure each node contains
		 * length size memory.
		 */
		ret = mbind(memory[i], length, MPOL_BIND, nmask, MAXNODES, 0);
		if (ret == -1)
			tst_brk(TBROK|TERRNO, "mbind");
#endif

		memset(memory[i], 10, length);

		if (mlock(memory[i], length))
			tst_res(TWARN | TERRNO, "mlock() failed");
	}

	SAFE_FILE_PRINTF(PATH_KSM "sleep_millisecs", "0");
	SAFE_FILE_PRINTF(PATH_KSM "pages_to_scan", "%ld",
			 nr_pages * num_nodes);
	/*
	 * merge_across_nodes and max_page_sharing setting can be changed
	 * only when there are no ksm shared pages in system, so set run 2
	 * to unmerge pages first, then to 1 after changing merge_across_nodes,
	 * to remerge according to the new setting.
	 */
	SAFE_FILE_PRINTF(PATH_KSM "run", "2");
	if (access(PATH_KSM "max_page_sharing", F_OK) == 0)
		SAFE_FILE_PRINTF(PATH_KSM "max_page_sharing",
			"%ld", nr_pages * num_nodes);
	tst_res(TINFO, "Start to test KSM with merge_across_nodes=1");
	SAFE_FILE_PRINTF(PATH_KSM "merge_across_nodes", "1");
	SAFE_FILE_PRINTF(PATH_KSM "run", "1");
	ksm_group_check(1, 1, nr_pages * num_nodes - 1, 0, 0, 0,
			nr_pages * num_nodes);

	SAFE_FILE_PRINTF(PATH_KSM "run", "2");
	tst_res(TINFO, "Start to test KSM with merge_across_nodes=0");
	SAFE_FILE_PRINTF(PATH_KSM "merge_across_nodes", "0");
	SAFE_FILE_PRINTF(PATH_KSM "run", "1");
	ksm_group_check(1, num_nodes, nr_pages * num_nodes - num_nodes,
			0, 0, 0, nr_pages * num_nodes);

	SAFE_FILE_PRINTF(PATH_KSM "run", "2");

	for (i = 0; i < num_nodes; i++)
		SAFE_MUNMAP(memory[i], length);

	free(memory);
}

static void setup(void)
{
	if (access(PATH_KSM "merge_across_nodes", F_OK) == -1)
		tst_brk(TCONF, "no merge_across_nodes sysfs knob");

	if (!is_numa(NULL, NH_MEMS, 2))
		tst_brk(TCONF, "The case needs a NUMA system.");

	if (n_opt)
		nr_pages = SAFE_STRTOUL(n_opt, 0, ULONG_MAX);

	/* save the current value */
	SAFE_FILE_SCANF(PATH_KSM "run", "%d", &run);
	SAFE_FILE_SCANF(PATH_KSM "merge_across_nodes",
			"%d", &merge_across_nodes);
	SAFE_FILE_SCANF(PATH_KSM "sleep_millisecs",
			"%d", &sleep_millisecs);
}

static void cleanup(void)
{
	if (merge_across_nodes != -1) {
		FILE_PRINTF(PATH_KSM "merge_across_nodes",
			    "%d", merge_across_nodes);
	}

	if (sleep_millisecs != -1)
		FILE_PRINTF(PATH_KSM "sleep_millisecs", "%d", sleep_millisecs);

	if (run != -1)
		FILE_PRINTF(PATH_KSM "run", "%d", run);
}

static struct tst_test test = {
	.needs_root = 1,
	.options = (struct tst_option[]) {
		{"n:", &n_opt,  "Allocate x pages memory per node"},
		{}
	},
	.setup = setup,
	.cleanup = cleanup,
	.save_restore = (const char * const[]) {
		"?/sys/kernel/mm/ksm/max_page_sharing",
		NULL,
	},
	.test_all = test_ksm,
};

#else
	TST_TEST_TCONF(NUMA_ERROR_MSG);
#endif
