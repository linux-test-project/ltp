/*
 * Copyright (C) 2010-2017  Red Hat, Inc.
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
 *
 * Kernel Samepage Merging (KSM) for Memory Resource Controller
 *
 * Basic tests were to start several programs with same and different
 * memory contents and ensure only to merge the ones with the same
 * contents. When changed the content of one of merged pages in a
 * process and to the mode "unmerging", it should discard all merged
 * pages there. Also tested it is possible to disable KSM. There are
 * also command-line options to specify the memory allocation size, and
 * number of processes have same memory contents so it is possible to
 * test more advanced things like KSM + OOM etc.
 *
 * Prerequisites:
 *
 * 1) ksm and ksmtuned daemons need to be disabled. Otherwise, it could
 *    distrub the testing as they also change some ksm tunables depends
 *    on current workloads.
 *
 * The test steps are:
 * - Check ksm feature and backup current run setting.
 * - Change run setting to 1 - merging.
 * - 3 memory allocation programs have the memory contents that 2 of
 *   them are all 'a' and one is all 'b'.
 * - Check ksm statistics and verify the content.
 * - 1 program changes the memory content from all 'a' to all 'b'.
 * - Check ksm statistics and verify the content.
 * - All programs change the memory content to all 'd'.
 * - Check ksm statistics and verify the content.
 * - Change one page of a process.
 * - Check ksm statistics and verify the content.
 * - Change run setting to 2 - unmerging.
 * - Check ksm statistics and verify the content.
 * - Change run setting to 0 - stop.
 */

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mem.h"

static int merge_across_nodes;

static struct tst_option ksm_options[] = {
	{"n:", &opt_numstr,  "-n       Number of processes"},
	{"s:", &opt_sizestr, "-s       Memory allocation size in MB"},
	{"u:", &opt_unitstr, "-u       Memory allocation unit in MB"},
	{NULL, NULL, NULL}
};

static void verify_ksm(void)
{
	int size = 128, num = 3, unit = 1;

	check_ksm_options(&size, &num, &unit);
	write_memcg();
	create_same_memory(size, num, unit);
}

static void setup(void)
{
	if (access(PATH_KSM, F_OK) == -1)
		tst_brk(TCONF, "KSM configuration is not enabled");

	if (access(PATH_KSM "merge_across_nodes", F_OK) == 0) {
		SAFE_FILE_SCANF(PATH_KSM "merge_across_nodes",
				"%d", &merge_across_nodes);
		SAFE_FILE_PRINTF(PATH_KSM "merge_across_nodes", "1");
	}

	save_max_page_sharing();
	mount_mem("memcg", "cgroup", "memory", MEMCG_PATH, MEMCG_PATH_NEW);
}

static void cleanup(void)
{
	if (access(PATH_KSM "merge_across_nodes", F_OK) == 0)
		FILE_PRINTF(PATH_KSM "merge_across_nodes",
				 "%d", merge_across_nodes);

	restore_max_page_sharing();

	umount_mem(MEMCG_PATH, MEMCG_PATH_NEW);
}

static struct tst_test test = {
	.tid = "ksm03",
	.needs_root = 1,
	.forks_child = 1,
	.options = ksm_options,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_ksm,
	.min_kver = "2.6.32",
};
