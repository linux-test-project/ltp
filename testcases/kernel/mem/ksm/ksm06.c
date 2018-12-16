/*
 * Copyright (C) 2013-2017  Red Hat, Inc.
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 */

/*
 * The case is designed to test new sysfs boolean knob
 * /sys/kernel/mm/ksm/merge_across_nodes, which was introduced by
 * commit 90bd6fd31c8097ee (ksm: allow trees per NUMA node).
 * when merge_across_nodes is set to zero only pages from the same
 * node are merged, otherwise pages from all nodes can be merged
 * together.
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
static unsigned long nr_pages;

static char *n_opt;
static struct tst_option ksm_options[] = {
	{"n:", &n_opt,  "-n x    Allocate x pages memory per node"},
	{NULL, NULL, NULL}
};

static const char * const save_restore[] = {
	"?/sys/kernel/mm/ksm/max_page_sharing",
	NULL,
};

static void test_ksm(void)
{
	if (n_opt)
		nr_pages = SAFE_STRTOUL(n_opt, 0, ULONG_MAX);
	else
		nr_pages = 100;

	test_ksm_merge_across_nodes(nr_pages);
}

static void setup(void)
{
	if (access(PATH_KSM "merge_across_nodes", F_OK) == -1)
		tst_brk(TCONF, "no merge_across_nodes sysfs knob");

	if (!is_numa(NULL, NH_MEMS, 2))
		tst_brk(TCONF, "The case needs a NUMA system.");

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
	.options = ksm_options,
	.setup = setup,
	.cleanup = cleanup,
	.save_restore = save_restore,
	.test_all = test_ksm,
};

#else
	TST_TEST_TCONF(NUMA_ERROR_MSG);
#endif
