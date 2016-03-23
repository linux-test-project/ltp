/*
 * Copyright (C) 2013 Linux Test Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
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
#include <errno.h>
#include <fcntl.h>
#if HAVE_NUMAIF_H
#include <numaif.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "numa_helper.h"
#include "test.h"
#include "safe_macros.h"
#include "mem.h"

char *TCID = "ksm06";
int TST_TOTAL = 1;

#if HAVE_NUMA_H && HAVE_LINUX_MEMPOLICY_H && HAVE_NUMAIF_H \
	&& HAVE_MPOL_CONSTANTS

static int run;
static int sleep_millisecs;
static int merge_across_nodes;
static int n_flag;
static unsigned long nr_pages;

static char *n_opt;
option_t options[] = {
	{ "n:", &n_flag, &n_opt },
	{ NULL, NULL, NULL }
};
static void usage(void);

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, options, &usage);

	if (n_flag)
		nr_pages = SAFE_STRTOUL(NULL, n_opt, 0, ULONG_MAX);
	else
		nr_pages = 100;

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		test_ksm_merge_across_nodes(nr_pages);
	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	if (access(PATH_KSM "merge_across_nodes", F_OK) == -1)
		tst_brkm(TCONF, NULL, "no merge_across_nodes sysfs knob");

	if (!is_numa(NULL, NH_MEMS, 2))
		tst_brkm(TCONF, NULL, "The case needs a NUMA system.");

	/* save the current value */
	SAFE_FILE_SCANF(NULL, PATH_KSM "run", "%d", &run);
	SAFE_FILE_SCANF(NULL, PATH_KSM "merge_across_nodes",
			"%d", &merge_across_nodes);
	SAFE_FILE_SCANF(NULL, PATH_KSM "sleep_millisecs",
			"%d", &sleep_millisecs);

	save_max_page_sharing();

	tst_sig(FORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;
}

void cleanup(void)
{
	FILE_PRINTF(PATH_KSM "merge_across_nodes",
			 "%d", merge_across_nodes);
	FILE_PRINTF(PATH_KSM "sleep_millisecs",
			 "%d", sleep_millisecs);
	FILE_PRINTF(PATH_KSM "run", "%d", run);

	restore_max_page_sharing();
}

static void usage(void)
{
	printf("  -n x    Allocate x pages memory per node\n");
}

#else /* no NUMA */
int main(void)
{
	tst_brkm(TCONF, NULL, "no NUMA development packages installed.");
}
#endif
